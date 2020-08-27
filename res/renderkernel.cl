#define DIM 512 //output resolution
//#define max_bounds 100 //of input


char get_sample_data(__global const char *A, const float3 sampling_position, int max_bounds){
	float3 sample = (float3) round(max(float3(0.0),(float3)min(float(max_bounds)-1,sampling_position)));//map to 0-max_bounds
	return A[uint(sample.z)*max_bounds*max_bounds+uint(sample.y)*max_bounds+uint(sample.x)];
}

//__read_only image3d_t
__kernel void matMult(int max_bounds,
					  __global char *A,
                      __global char *C//,
				//	  __read_only image3d_t img
							 ) {
	const uint x = get_global_id(0);//spalte
	const uint y = get_global_id(1);//zeile
	// the traversal loop,
	// termination when the sampling position is outside volume boundarys
	// another termination condition for early ray termination is added
	const float3 ray_entry_position = (float3)(x*max_bounds/DIM, y*max_bounds/DIM,0);//map to 0-1
	const float3 camera_location = (float3)(x*max_bounds/DIM,y*max_bounds/DIM,-1);//map to 0-1
	const float sampling_distance = 1;
	float3 ray_increment = normalize(ray_entry_position - camera_location) * sampling_distance;
	float3 sampling_pos = ray_entry_position+ray_increment;
	float dst = 0.0;
	float trsp = 1;
	float brightnes = 1;
	while (all(sampling_pos >= 0) && all(sampling_pos < max_bounds)) {
		// get sample
		char s = get_sample_data(A, sampling_pos.xyz, max_bounds);
		//bool s = bool(read_imagei(A, sampling_pos).a);
		
		//early termination
		if (trsp <= 0.1 || dst>1.0){
			dst = min((float)1.0,dst);
			break;
		}

		#if ENABLE_LIGHTNING == 1 // Add Shading
			color *= get_shading(sampling_pos, ray_increment);
		#endif
		
		#if ENABLE_OPACITY_CORRECTION == 1 // Opacity Correction
			color.a = 1 - pow(1.0 - color.a, sampling_distance/ sampling_distance_ref*300 );
		#endif
		
		//front-to-back
		// accumulate color
		if (s==1){
			dst += 0.04 * trsp;
			// update opacity
			trsp *= 0.975;
		}
		
		// increment the ray sampling position
		sampling_pos += ray_increment;
	}
	C[y*DIM+x] = dst*255;
}
