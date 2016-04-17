#include "Triangle.h"

//=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~//
// Triangle object.                                                       //
//                                                                        //
// The triangle object is defined by three vertices in R3.  This is a     //
// simple flat triangle with no normal vector interpolation.  The         //
// triangle structure is defined to accommodate the barycentric coord     //
// method of intersecting a ray with a triangle.                          //
//=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~//

Triangle::Triangle( const Vec3 &A_, const Vec3 &B_, const Vec3 &C_ )
    {
	float base;		// Length of the base of the triangle
	float height;	// Height of the triangle
	Vec3 N_pos;		// Normal with all values positive

	// Assign values to the corner points
	A = A_;
	B = B_;
	C = C_;

	// Compute the normal to plane of the triangle
	// Cross product between two vectors created by points fo the triangle

    N = Unit( ( C - B ) ^ ( A - B ) );

	// Compute the distance from origin to plane of triangle.
	// This equals to D on the plane coordinates

    d = -( A.x * N.x + A.y * N.y + A.z * N.z );

	// Computes the bounding box;

	// Initiallizes the box coordinates
    box.X.min = box.X.min  = A.x;
    box.Y.min = box.Y.min  = A.y;
    box.Z.min = box.Z.min  = A.z;
	// Check B coordinates
	if( B.x < box.X.min ) box.X.min = B.x;
	else if( B.x > box.X.max ) box.X.max = B.x;
	if( B.y < box.Y.min ) box.Y.min = B.y;
	else if( B.y > box.Y.max ) box.Y.max = B.y;
	if( B.z < box.Z.min ) box.Z.min = B.z;
	else if( B.z > box.Z.max ) box.Z.max = B.z;
	// Check C coordinates
	if( C.x < box.X.min ) box.X.min = C.x;
	else if( C.x > box.X.max ) box.X.max = C.x;
	if( C.y < box.Y.min ) box.Y.min = C.y;
	else if( C.y > box.Y.max ) box.Y.max = C.y;
	if( C.z < box.Z.min ) box.Z.min = C.z;
	else if( C.z > box.Z.max ) box.Z.max = C.z;

	// Compute absolute value for all components of the normal
	N_pos = N;
	N_pos.x = fabs( N_pos.x );
	N_pos.y = fabs( N_pos.y );
	N_pos.z = fabs( N_pos.z );
	
	if( N_pos.x >= N_pos.y && N_pos.x >= N_pos.z )
		axis = 0;
	else if( N_pos.y >= N_pos.x && N_pos.y >= N_pos.z )
		axis = 1;
	else if( N_pos.z >= N_pos.x && N_pos.z >= N_pos.y )
		axis = 2;
	
	// Compute the area

	// First compute the base of the triangle
	base = (float) dist( B, A );

	// Then compute the height of the triangle
	// It is computed as an point to line distance
	height = (float) Length( ( B - A ) ^ ( A - C ) ) / (float) Length( B - A );

	// Therefore, the area is:
	area = base * height * 0.5f;

    next = NULL;
    }

Object *Triangle::ReadString( const char *params ) // Read params from string.
    {
    float ax, ay, az, bx, by, bz, cx, cy, cz;
    if( sscanf( params, "triangle (%f,%f,%f) (%f,%f,%f) (%f,%f,%f)", 
        &ax, &ay, &az, &bx, &by, &bz, &cx, &cy, &cz ) == 9 )
        return new Triangle( Vec3( ax, ay, az ), Vec3( bx, by, bz ), Vec3( cx, cy, cz ) );
    return NULL;
    }

Box3 Triangle::GetBounds() const // Return pre-computed box.
    {
    return box;
    }



bool Triangle::Intersect( const Ray &ray, HitGeom &hitgeom ) const
    {
		Plane plane;
		Vec3 aux;
		double dist;
		Vec3 punt;

		plane.A = N.x;
		plane.B = N.y;
		plane.C = N.z;
		plane.D = d;

		dist = plane.Intersect(ray);

		if (dist > 0.0 && dist < hitgeom.distance) {

			float t = -(N*(ray.origin) + d) / (N*(ray.direction));

			if (t > 0)
			{
				//Calculem el punt d'intersecció 
				punt = ray.origin + ray.direction * t;


				Mat3x3 matriu;
				switch (axis)
				{
				case 0:
					punt.x = 1;
					matriu.m[0][0] = 1;
					matriu.m[0][1] = 1;
					matriu.m[0][2] = 1;
					matriu.m[1][0] = A.y;
					matriu.m[1][1] = B.y;
					matriu.m[1][2] = C.y;
					matriu.m[2][0] = A.z;
					matriu.m[2][1] = B.z;
					matriu.m[2][2] = C.z;

					break;
				case 1:
					punt.y = 1;
					matriu.m[0][0] = A.x;
					matriu.m[0][1] = B.x;
					matriu.m[0][2] = C.x;
					matriu.m[1][0] = 1;
					matriu.m[1][1] = 1;
					matriu.m[1][2] = 1;
					matriu.m[2][0] = A.z;
					matriu.m[2][1] = B.z;
					matriu.m[2][2] = C.z;
					break;
				case 2:
					punt.z = 1;
					matriu.m[0][0] = A.x;
					matriu.m[0][1] = B.x;
					matriu.m[0][2] = C.x;
					matriu.m[1][0] = A.y;
					matriu.m[1][1] = B.y;
					matriu.m[1][2] = C.y;
					matriu.m[2][0] = 1;
					matriu.m[2][1] = 1;
					matriu.m[2][2] = 1;
					break;
				}

				const Mat3x3 m = Mat3x3(matriu);


				Mat3x3 minv = (1 / det(m)) * Transpose(Adjoint(m));
				Vec3 coordBaricentriques = minv * punt;

				//Si les coordBaricentriques estan entre 0 i 1, omplim hitgeom
				if ((coordBaricentriques.x > 0 && coordBaricentriques.x < 1) && (coordBaricentriques.y > 0 && coordBaricentriques.y < 1) && (coordBaricentriques.z > 0 && coordBaricentriques.z < 1)) {

					hitgeom.distance = dist;
					hitgeom.normal = N;
					hitgeom.origin = ray.origin;
					hitgeom.point = punt;

					return true;
				}

			}



		}

		return false;
}
			/*Vec3 position = ray.origin + (ray.direction * dist);

			double as_x = position.x - A.x;
			double as_y = position.y - A.y;

			bool s_ab = (B.x - A.x)*as_y - (B.y - A.y)*as_x > 0;

			if ((C.x - A.x)*as_y - (C.y - A.y)*as_x > 0 == s_ab) return false;

			if ((C.x - B.x)*(position.y - B.y) - (C.y - B.y)*(position.x - B.x) > 0 != s_ab) return false;

			hitgeom.distance = dist;
			hitgeom.point = position;
			hitgeom.normal = N;
			hitgeom.origin = ray.origin;
			*/


Sample Triangle::GetSample( const Vec3 &P, const Vec3 &N_point ) const
{
	double s, t;
	Vec3 st;
	Sample sample;
	double d = 1.0;
	double cos_theta, cos_theta2;

	s = rand(0.0, 1.0);
	t = rand(0.0, 1.0);

	sample.P = (1.0 - sqrt(s))*A + (sqrt(s)*(1 - t))*B + (t*sqrt(s))*C;
	Vec3 center = Vec3((A.x + B.x + C.x / 3), (A.y + B.y + C.y / 3), (A.z + B.z + C.z / 3));
	Vec3 SV = Unit((P - sample.P));
	//Vec3 N = -N_point;
	//cos_theta = fabs(N * Unit(P - sample.P));
	cos_theta = fabs(N.x * SV.x + N.y * SV.y + N.z * SV.z);
	cos_theta2 = fabs(N*Unit(P-sample.P));
	// Calcule the distance between the sample and the point P
	d = Length(P - sample.P);
	// Assigns the weight
	sample.w = area*((cos_theta2))/ (d * d);
	sample.w = sample.w > 2 * Pi ? 2 * Pi : sample.w;
	
	return sample;
}