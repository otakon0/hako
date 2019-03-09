#include <iostream>
#define N 4

void print_mat(double mat[N][N],int size){
	for(int i=0;i<N;++i){
		for(int j=0;j<N;++j){
			printf("%3.2lf ",mat[i][j]);
		}
		printf("\n");
	}
}

void print_mat(double **mat,int size){
	for(int i=0;i<N;++i){
		for(int j=0;j<N;++j){
			printf("%3.2lf ",mat[i][j]);
		}
		printf("\n");
	}
}
int main(int argc,char **argv){

	double A[N][N] = {
		{8.0,16.0,24.0,32.0},
		{2.0,7.0,12.0,17.0},
		{6.0,17.0,32.0,59.0},
		{7.0,22.0,46.0,105.0}
	};
	double **matA = new double*[N];
	for(int i=0;i<N;++i) matA[i] = new double[N];
	double **L = new double*[N];
	for(int i=0;i<N;++i) L[i] = new double[N];
	double **U = new double*[N];
	for(int i=0;i<N;++i) U[i] = new double[N];

	for(int i=0;i<N;++i){
		for(int j=0;j<N;++j){
			matA[i][j] = A[i][j];
			L[i][j]    = 0.0;
			if(i==j){
				U[i][j] = 1.0;
			}else{
				U[i][j] = 0.0;
			}
		}
	}

	/* 以下、LU分解 */
	for(int i=0;i<N;++i){
		double l_00 = matA[i][i];
		L[i][i] = l_00;
		/* copy the l column from matA */
		for(int j=i+1;j<N;++j){
			L[j][i] = matA[j][i];
			U[i][j] = matA[i][j] / l_00;
		}

		for(int j=i+1;j<N;++j){
			double l_low = L[j][i];
			for(int k=i+1;k<N;++k){
				matA[j][k] = matA[j][k] - l_low * U[i][k];
			}
		}
	}

	printf("\nMatrix A\n");
	print_mat(A,N);
	printf("Matrix L\n");
	print_mat(L,N);
	printf("Matrix U\n");
	print_mat(U,N);

	for(int i=0;i<N;++i) delete [] matA[i];
	for(int i=0;i<N;++i) delete [] L[i];
	for(int i=0;i<N;++i) delete [] U[i];
	delete [] matA;
	delete [] L,U;
	return 0;
}
