#include <math.h>
#include <algorithm>
/***************************************************************************
*                                                                          *
* This is the source file for a ray tracer. It defines most of the		   *
* fundamental functions using in ray tracing.  In particular, "Trace" and  *
* "Shade".  It also defines the MakeImage function, which casts all the    *
* rays from the eye, and several object intersection methods.  Some of the *
* functions are left blank, or with "place holders" that don't do very     *
* much.  You need to fill these in.                                        *
*                                                                          *
*                                                                          *
***************************************************************************/

static const int tree_depth = 3;		// Number of recursions to compute indirect illumination
static const int rays_pixel = 3;

#include "Raytracer.h"

// Draw image on the screen
void Raytracer::draw( void )
{
	glDrawPixels( resolutionX, resolutionY, GL_RGB, GL_UNSIGNED_BYTE, &(*I)( 0 , 0 ) );
}

// Cast_line casts all the initial rays starting from the eye for a single
//  raster line. Copies pixels to image object.
void Raytracer::cast_line(World world)
{
	Ray ray;
	Color color;	// Color computed when using multiple rays per pixel

	ray.origin = world.getCamera().eye; // All initial rays originate from the eye.
	ray.no_emitters = false;

	Vec3 G = Unit(world.getCamera().lookat - world.getCamera().eye);	// Gaze direction.
	Vec3 U = Unit(world.getCamera().up / G);							// Up vector.
	Vec3 R = Unit(G ^ U);											// Right vector.
	Vec3 O = (world.getCamera().vpdist * G) - R + U;					// "Origin" for the raster.
	Vec3 dU = U * (2.0 / (resolutionY - 1));						// Up increments.
	Vec3 dR = R * (2.0 / (resolutionX - 1));						// Right increments.

	if (currentLine % 10 == 0) cout << "line " << currentLine << endl;
	for (int i = 0; i < resolutionX; i++)
	{
		if (rays_pixel == 1)
		{
			// One ray per pixel
			ray.direction = Unit(O + i * dR - currentLine * dU);
			color = Trace(ray, world.getScene(), tree_depth);
		}
		else
		{
			// Multisampling
			for (int n = 0; n < rays_pixel; n++)
			{
				ray.direction = Unit(O + (i + rand(0.0, 1.0) - 0.5) * dR - (currentLine + rand(0.0, 1.0) - 0.5) * dU);
				color += Trace(ray, world.getScene(), tree_depth);
			}
		}
		(*I)(resolutionY - currentLine - 1, i) = ToneMap(color / rays_pixel);

		color.blue = 0;
		color.green = 0;
		color.red = 0;
	}

	if (++currentLine == resolutionY)
	{
		// Image computation done, save it to file
		cout << "done." << endl;
		I->Write("Resultat.ppm");
		isDone = true;
	}
}


// This is a trivial tone mapper; it merely maps values that are
// in [0,1] and maps them to integers between 0 and 255.  If the
// real value is above 1, it merely truncates.  A true tone mapper
// would attempt to handle very large values nicely, without
// truncation; that is, it would try to compensate for the fact that
// displays have a very limited dynamic range.
Pixel Raytracer::ToneMap( const Color &color )
{
	int red   = (int)floor( 256 * color.red   );
    int green = (int)floor( 256 * color.green );
    int blue  = (int)floor( 256 * color.blue  );
    channel r = (channel)( red   >= 255 ? 255 : red   ); 
    channel g = (channel)( green >= 255 ? 255 : green ); 
    channel b = (channel)( blue  >= 255 ? 255 : blue  );
    return Pixel( r, g, b );
}

// Trace is the most fundamental of all the ray tracing functions.  It
// answers the query "What color do I see looking along the given ray
// in the current scene?"  This is an inherently recursive process, as
// trace may again be called as a result of the ray hitting a reflecting
// object.  To prevent the possibility of infinite recursion, a maximum
// depth is placed on the resulting ray tree.
Color Raytracer::Trace( const Ray &ray, const Scene &scene, int max_tree_depth  )
{
    Color   color;                    // The color to return.
    HitInfo hitinfo;                  // Holds info to pass to shader.

	// Intitallizes hit distance to infinity to allow finding intersections in all ray length
	hitinfo.geom.distance = Infinity;

	if (Cast( ray, scene, hitinfo ) > 0.0f && max_tree_depth > -1 )
	{
        // The ray hits an object, so shade the point that the ray hit.
        // Cast has put all necessary information for Shade in "hitinfo".
		
		// If the ray has no_emitters activated and the first hit is an emitter
		//  this ray shouldn't contribute to the color of the current pixel
		if( hitinfo.material.Emitter() && ray.no_emitters == true ) color = Color ();

		// The ray hits an object, so shade the point that the ray hit.
        // Cast has put all necessary information for Shade in "hitinfo".
		else color = Shade( hitinfo, scene, max_tree_depth  );
    }
    else
    {
        // Either the ray has failed to hit anything, or
        // the recursion has bottomed out.
        color = scene.bgcolor;
    }
    
    return color;
}

// Cast finds the first point of intersection (if there is one)
// between a ray and a list of geometric objects.  If no intersection
// exists, the function returns false.  Information about the
// closest object hit is returned in "hitinfo". 
int Raytracer::Cast( const Ray &ray, const Scene &scene, HitInfo &hitinfo, Object *ignore )
{
	int hit = false;

    // Each intersector is ONLY allowed to write into the "HitGeom"
    // structure if it has determined that the ray hits the object
    // at a CLOSER distance than currently recorded in HitGeom.distance.
    // When a closer hit is found, the material fields of the "HitInfo"
    // structure are updated to hold the material of the object that 
    // was just hit.

    for( Object *object = scene.first; object != NULL; object = object->next )
    {
        if( object != ignore && object->Intersect( ray, hitinfo.geom ) )
            {
            hitinfo.material = object->material;  // Material of closest surface.
            hit = true;                           // We have hit an object.
            }
    }
    return hit;
}

// Returns a sample into the project hemishpere. This is a type of importance sampling.
Sample Raytracer::SampleProjectedHemisphere(const Vec3 &N) {

	double s, t;
	Sample sample;

	double r = 1.0;
	s = rand(0.0, 1.0);
	t = rand(0.0, 1.0);

	//Punt aleatori a la semiesfera.
	double x = sqrt(t)*cos(2 * Pi*s);
	double y = sqrt(t)*sin(2 * Pi*s);
	double z = sqrt(1 - x*x - y*y);

	//Obtenim un punt aleatori de la nostra esfera.
	Vec3 reflected_sample = Vec3(x, y, z);
	sample.P = (Reflection(-reflected_sample, Unit( N + Vec3(0, 0, 1))));

	// Assigns the weight
	sample.w = Pi;

	return sample;
}

// Returns a sample into the specular lobe. This is a type of importance sampling
Sample Raytracer::SampleSpecularLobe(const Vec3 &R, float phong_exp) {
	double s, t;
	Sample sample;

	double r = 1.0;
	s = rand(0.0, 1.0);
	t = rand(0.0, 1.0);

	//Punt aleatori a la semiesfera.
	double x = sqrt(1 - pow(t, 2 / (phong_exp + 1)))* cos(2 * Pi*s);
	double y = sqrt(1 - pow(t, 2 / (phong_exp + 1)))* sin(2 * Pi*s);
	double z = sqrt(1 - x*x - y*y);

	//Obtenim un punt aleatori de la nostra esfera.
	Vec3 reflected_sample = Vec3(x, y, z);
	sample.P = (Reflection(-reflected_sample , Unit(R + Vec3(0,0,1))));

	// Assigns the weight
	sample.w = Pi;

	return sample;
}



// Shade assigns a color to a point on a surface, as it is seen
// from another point.  The coordinates of these points, the normal
// of the surface, and the surface material are all recorded in the
// HitInfo structure.  The shader will typically make calls to Trace
// to handle shadows and reflections.
Color Raytracer::Shade(const HitInfo &hit, const Scene &scene, int max_tree_depth)
{
	Color color = { 0.0f, 0.0f, 0.0f };
	Vec3 point = hit.geom.point + hit.geom.normal*Epsilon;
	HitInfo hitObstacle = hit;
	Ray ray, ray2;
	Color diffuse = Color(0,0,0), specular = Color(0,0,0);
	Color direct = Color(0,0,0), indirect = Color(0,0,0);
	Sample lightPoint;

	Vec3 V;
	Color irradiant = Color(0,0,0);
	int espec = 0;
	Color indirect_hemisphere = Color(0, 0, 0), indirect_lobe = Color(0, 0, 0);

	//if (max_tree_depth >= 0) {

		if (hit.material.Emitter()) {
			return hit.material.m_Diffuse;
		}else{
			for (Object *object = scene.first; object != NULL; object = object->next)
			{

				if (object->material.Emitter() == true)
				{
					lightPoint = object->GetSample(hit.geom.point, hit.geom.normal);
					ray.origin = hit.geom.point;

					//lightPoint.P = lightPoint.P + hit.geom.normal*Epsilon;
					ray.direction = Unit(lightPoint.P - hit.geom.point);
					hitObstacle.geom.distance = Length(lightPoint.P - point);

					V = Unit(hit.geom.origin - hit.geom.point);
						Vec3 N = (hit.geom.normal);
						Vec3 H = Unit(V + ray.direction);
						//V = Unit(hit.geom.origin - hit.geom.point);
						Vec3 L = (-ray.direction);// Unit(point - lightPoint.P);
						Vec3 R = Unit(Reflection(-L, N));//Unit((2*N*(N*L))-L);

					espec = hit.material.m_Phong_exp;

					double c = (L*H);
					double g = sqrt((pow(1.4 / 1, 2.0))+ pow(c,2)-1);
					Color f0 = hit.material.m_Specular;


					double F = (f0.red + (1.0 - f0.red, 1.0 - f0.green, 1.0 - f0.blue)*(pow((1 - (N*V)), 5)), f0.green + (1.0 - f0.red, 1.0 - f0.green, 1.0 - f0.blue)*(pow((1 - (N*V)), 5)), f0.blue+(1.0 - f0.red, 1.0 - f0.green, 1.0 - f0.blue)*(pow((1 - (N*V)), 5)));
					//double F = (1 / 2)*((pow(g - c, 2))/(pow(g + c, 2)))*(1+pow((c*(g+c)-1),2)/ (1 + pow((c*(g - c) + 1), 2)));
					double D = ((espec + 2) / (2 * Pi))*(pow((H*N), espec));
					//double D = 1/(sqrt(2*Pi))
					double G = min(min(1.0, (2 * (N*H)*(N*V)) / (V*H)), (2 * (N*H)*(N*L)) / (L*H));
					if (G < 0) G = 0;
					if (D < 0) D = 0;
					if (F < 0) F = 0;

					if (!Cast(ray, scene, hitObstacle)) {
						

						if (N * L > 0) {
							diffuse = (N*L)*hit.material.m_Diffuse/Pi;
						}
						else {
							diffuse = Color(0.0, 0.0, 0.0);
						}

						irradiant = object->material.m_Emission*(lightPoint.w);
						
						if ((R*V) > 0 && hit.material.m_Phong_exp > 0) {
							specular = /*(espec + 2)/(2*Pi) * pow((R*V), espec)*/(F*D*G / (4 * (N*L)*(N*V)))*hit.material.m_Specular;
						}
						else {
							specular = Color(0.0, 0.0, 0.0);
						}
						direct += (diffuse + specular)*irradiant;
					}
				}
			}
			if (hit.material.m_Phong_exp > 0) {
				Vec3 N = (hit.geom.normal);
				Vec3 L = Unit(-ray.direction);
				V = -Unit(hit.geom.point - hit.geom.origin);
				Vec3 R = Unit(Reflection(-V, N));
				Vec3 H = Unit(V + ray.direction);
				double c = (L*H);
				double g = sqrt((pow(1.4 / 1, 2.0)) + pow(c, 2) - 1);
				Color f0 = hit.material.m_Specular;

				double F = (f0.red + (1.0 - f0.red, 1.0 - f0.green, 1.0 - f0.blue)*(pow((1 - (N*V)), 5)), f0.green + (1.0 - f0.red, 1.0 - f0.green, 1.0 - f0.blue)*(pow((1 - (N*V)), 5)), f0.blue + (1.0 - f0.red, 1.0 - f0.green, 1.0 - f0.blue)*(pow((1 - (N*V)), 5)));
				//double F = (1 / 2)*((pow(g - c, 2))/(pow(g + c, 2)))*(1+pow((c*(g+c)-1),2)/ (1 + pow((c*(g - c) + 1), 2)));
				double D = ((espec + 2) / (2 * Pi))*(pow((H*N), espec));
				double G = min(min(1.0, (2 * (N*H)*(N*V)) / (V*H)), (2 * (N*H)*(N*L)) / (L*H));
				if (G < 0) G = 0;
				if (D < 0) D = 0;
				if (F < 0) F = 0;


				float phong_exp = hit.material.m_Phong_exp;
				ray2.origin = point;

				Sample sample_hemisphere = SampleProjectedHemisphere(N);
				ray2.direction = Unit(sample_hemisphere.P);
				//ray2.no_emitters = true;
				indirect_hemisphere = Trace(ray2, scene, max_tree_depth-1)*  hit.material.m_Diffuse;

				Sample specular_sample = SampleSpecularLobe(R, phong_exp);
				ray2.direction = Unit(specular_sample.P);
				indirect_lobe = Trace(ray2, scene, max_tree_depth - 1)*F*D*G / (4 * (N*L)*(N*V));// *hit.material.m_Specular;

				indirect = indirect_hemisphere + indirect_lobe;// *(F*D*G / (4 * (N*L)*(N*V)));
			}
			
			return (direct) + indirect;
		}
	//}
}

