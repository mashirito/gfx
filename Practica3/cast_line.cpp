void Raytracer::cast_line( World world )
{
    Ray ray;
	Color color;	// Color computed when using multiple rays per pixel

	ray.origin = world.getCamera().eye; // All initial rays originate from the eye.
	ray.no_emitters = false;

    Vec3 G  = Unit( world.getCamera().lookat - world.getCamera().eye );	// Gaze direction.
    Vec3 U  = Unit( world.getCamera().up / G );							// Up vector.
    Vec3 R  = Unit( G ^ U );											// Right vector.
    Vec3 O  = ( world.getCamera().vpdist * G ) - R + U;					// "Origin" for the raster.
    Vec3 dU = U * ( 2.0 / ( resolutionY - 1 ) );						// Up increments.
	Vec3 dR = R * ( 2.0 / ( resolutionX - 1 ) );						// Right increments.

    if( currentLine % 10 == 0 ) cout << "line " << currentLine << endl;
    for( int i = 0; i < resolutionX; i++ )
    {
		if( rays_pixel == 1 )
		{
			// One ray per pixel
			ray.direction = Unit( O + i * dR - currentLine * dU  );
			color = Trace( ray, world.getScene(), tree_depth );
		}
		else
		{
			// Multisampling
			for( int n = 0 ; n < rays_pixel ; n++ )
			{
				ray.direction = Unit( O + ( i + rand( 0.0 , 1.0 ) - 0.5 ) * dR - ( currentLine + rand( 0.0 , 1.0 ) - 0.5 ) * dU  );
				color += Trace( ray, world.getScene(), tree_depth );
			}
		}
		(*I)( resolutionY-currentLine-1, i ) = ToneMap( color / rays_pixel );

		color.blue = 0;
		color.green = 0;
		color.red = 0;
    }

	if (++currentLine == resolutionY)
	{
		// Image computation done, save it to file
		cout << "done." << endl;
	    I->Write( "Resultat.ppm" );
		isDone = true;
	}
}