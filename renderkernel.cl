/**Kernel Variante A1+A3**/
#define DIM 1000
__kernel void matMult(__global float *A,
                      __global float *C
							 ) {
	int i, j, k;
	i = get_global_id(0);//zeile

	//kopie in lokalen speicher
	float columnB[DIM];
	for (k = 0; k < DIM; k++) {
		columnB[k] = B[k*DIM+i];
	}

	for (j = 0; j < DIM; j++) {//loop Ÿber alle spalten
		for (k = 0; k < DIM; k++) {
			C[j*DIM+i] += A[j*DIM+k] * columnB[k];
		}
	}
}
