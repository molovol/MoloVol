#define DIM 512 //output resolution
#define max_bounds DIM //of input

bool inside_volume_bounds(const float3 sampling_pos){
	return all(sampling_pos >= (float3)(0))
            && all(sampling_pos <= max_bounds);
}

float get_sample_data(__global const float *A, const float3 sampling_position){
	return A[int(sampling_position.z)*DIM*DIM+int(sampling_position.y)*DIM+int(sampling_position.x)];
}

__kernel void matMult(__global float *A,
                      __global char *C
							 ) {
	int x, y, hit;
	x = get_global_id(0);//spalte
	y = get_global_id(1);//zeile
	for (int i=0;i<10000;i++){
		A[2000+i]=1;
	}
	// the traversal loop,
	// termination when the sampling position is outside volume boundarys
	// another termination condition for early ray termination is added
	float3 ray_entry_position = (float3)(x,y,0);
	float3 camera_location = (float3)(0);
	float sampling_distance = 1;
	float3 ray_increment = normalize(ray_entry_position - camera_location) * sampling_distance;
	float3 sampling_pos = ray_entry_position+ray_increment;
	float dst = 0;
	float trsp = 1;
	float brightnes = 1;
	while (inside_volume_bounds(sampling_pos)) {
		// get sample
		float s = get_sample_data(A, sampling_pos);
		
		//early termination
		if (trsp <= 0.1){
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
		dst += s * trsp;

		// update opacity
		trsp *= 0.8;

		// increment the ray sampling position
		sampling_pos += ray_increment;
	}
	C[y*DIM+x] = dst*255;
}
