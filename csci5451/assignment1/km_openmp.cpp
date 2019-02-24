/*
 * K-Means Algorithm with pthreads.
 */
#ifndef _POSIX_C_SOURCE
	#define _POSIX_C_SOURCE 199309L
#endif

#ifedf _OPENMP
#include <omp.h>
#endif

#include <time.h>

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <sstream>
#include <string>
#include <iomanip>

#define MAX_LOOP 20

static inline double monotonic_seconds();
static void print_time(const double seconds);
static inline double getDistance(double *point,double *centroid,int dim);
void readDataFromFile(const char *filepath);
void deletePointer();

int main(int argc,char *argv[]){

	if(argc != 4){
		printf("Usage %s k-meshdata number_of_cluster number_of_threads\n",argv[0]);
		return 0;
	}

	int g_numclusters;
	int g_numdimensions;
	int g_numthreads;
	double **g_dataList;

	g_numclusters = std::stoi(argv[2]);
	g_numthreads  = std::stoi(argv[3]);

	std::ifstream file(argv[1]);
	if(!file){
		printf("Failure to open %s\n",argv[1]);
		exit(0);
	}

	/* read data from the file given as an argument */
	{
		std::string line;
		getline(file,line);
		std::istringstream stream(line);
		stream >> g_numclusters >> g_numdimensions;
		printf("The number of clusters    : %d\n"
			   "The number of dimension   : %d\n",
		   			g_numclusters,g_numdimensions);

		g_dataList = new double*[g_numclusters];
		for(int i=0;i<g_numclusters;++i)
			g_dataList[i] = new double[g_numdimensions];

		int i=0,j=0;
		while(getline(file,line)){
			std::istringstream st(line);
			std::string token;
			while(getline(st,token,' ')){
				g_dataList[i][j] = std::stod(token);
				j++;
				if(j>=g_numdimensions) break;
			}
			i++;
			j = 0;
			if(i>=g_numclusters) break;
		}
	}
	file.close();


	for(int i=0;i<MAX_LOOP;++i){

		#pragma omp parallel
		{
			for(int j=0;j<g_numclusters;++j){
			if(j % 1000) printf("[%2d] thread[%2d] %6d ",i,omp_get_thread_num(),j);
			}

		#pragma omp single
		{
			printf("Reset");
		}

		}

	}


	for(int i=0;i<g_numclusters;++i){
		delete [] g_dataList[j];
	}
	delete [] g_dataList;


	return 0;
}
