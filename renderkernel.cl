#define DIM 512
__kernel void matMult(__global float *A,
                      __global char *C
							 ) {
	int x, y, hit;
	x = get_global_id(0);//spalte
	y = get_global_id(1);//zeile
	float res = cos((x*40+y*100)/float(DIM))/2+0.5;
	C[y*DIM+x] = res*255;

	
}
