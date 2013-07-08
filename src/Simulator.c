/*
 ============================================================================
 Name        : Simulator.c
 Author      : Alexander Winkler
 Version     :
 Copyright   : Whatever you want to do with it
 Description : CT Artefact Simulation: Inputs a slice and outputs a sinogram
 ============================================================================
 */

#include "Simulator.h"




int simulation(char *pathToSlice, char *pathToOutputSinogram) {
	logIt(DEBUG, "simulation(char *pathToSlice, char *pathToOutputSinogram) started.");
	time_t start;
	time_t stop;
	time(&start);
	double run = 0.0;
	logIt(INFO, "pathToOutputSinogram: %s", pathToOutputSinogram );
	logIt(INFO, "pathToSlice: %s", pathToSlice);

	setUpRawFiles(pathToSlice);


	int a = 0;


	allocateAllRaws();
	allocateUnsignedIntArray(&result, NUM_ANGLES, SINOGRAMSIZE);
	FILE *outFile = fopen(pathToOutputSinogram, "wb");


	(void)loadPGMToRaw(&ironRaw,	ironImage);
	(void)loadPGMToRaw(&boneRaw,	boneImage);
	(void)loadPGMToRaw(&waterRaw,	waterImage);
	(void)loadPGMToRaw(&airRaw,		airImage);
	(void)loadPGMToRaw(&muscleRaw,	muscleImage);
	(void)loadPGMToRaw(&tissueRaw,	tissueImage);

	logIt(INFO, "Starting simulation.");

	logIt(INFO, "Starting projection.");
		HANDLE hThread[4];
		DWORD threadID[4];
		t *arg[4];

		arg[0] = (t *)malloc(sizeof(t));
		arg[0]->data1 = 0;
		arg[0]->data2 = NUM_ANGLES/4;

		arg[1] = (t *)malloc(sizeof(t));
		arg[1]->data1 = NUM_ANGLES/4;
		arg[1]->data2 = NUM_ANGLES/2;

		arg[2] = (t *)malloc(sizeof(t));
		arg[2]->data1 = NUM_ANGLES/2;
		arg[2]->data2 = (3*NUM_ANGLES)/4;

		arg[3] = (t *)malloc(sizeof(t));
		arg[3]->data1 = (3*NUM_ANGLES)/4;
		arg[3]->data2 = NUM_ANGLES;

		hThread[0] = (HANDLE) CreateThread(NULL, 0, projectFromTo, (void *) arg[0], 0, &threadID[0]);
		hThread[1] = (HANDLE) CreateThread(NULL, 0, projectFromTo, (void *) arg[1], 0, &threadID[1]);
		hThread[2] = (HANDLE) CreateThread(NULL, 0, projectFromTo, (void *) arg[2], 0, &threadID[2]);
		hThread[3] = (HANDLE) CreateThread(NULL, 0, projectFromTo, (void *) arg[3], 0, &threadID[3]);

		logIt(DEBUG, "Waiting for Threads to finish.");
		WaitForMultipleObjects(4,hThread,TRUE,INFINITE);
		CloseHandle(hThread[0]);
		CloseHandle(hThread[1]);
		CloseHandle(hThread[2]);
		CloseHandle(hThread[3]);

//		projectFromTo((void *)arg[0]);
//		projectFromTo((void *)arg[1]);
//		projectFromTo((void *)arg[2]);
//		projectFromTo((void *)arg[3]);

//	for(a = 0; a<NUM_ANGLES; a++){
//		project(a);
//		logIt(INFO, "Projection %d of %d finished.", a, NUM_ANGLES);
//	}
	logIt(INFO, "Projection completed.");


	freeAllRaws();
	exportPGM(outFile, result, NUM_ANGLES, SINOGRAMSIZE);

	time(&stop);
	run = difftime(stop, start);


	logIt(INFO,  "Simulation finished. Runtime: %lf.", run);
	fclose(outFile);
	closeAllInputImages();
	logIt(DEBUG, "simulation(char *pathToSlice, char *pathToOutputSinogram) finished.");
	return EXIT_SUCCESS;
}

DWORD WINAPI projectFromTo(void * param){
	logIt(DEBUG, "projectFromTo(void *param) started.");
	t *args = (t*) param;
	int fromAngle = args->data1;
	int toAngle = args->data2;

	for(;fromAngle<toAngle; fromAngle++){
		project(fromAngle);
	}
	logIt(DEBUG, "projectFromTo(void *param) finished.");
	return 0;
}

void allocateUnsignedIntArray(unsigned int ***raw, int row, int col) {
	logIt(DEBUG, "allocateUnsignedIntArray(unsigned int ***raw, int row, int col) started.");
	int i = 0;

	*raw = malloc(row * sizeof(unsigned int *));

	if(*raw == 0){
		logIt(ERROR, "out of memory");
		fprintf(stderr, "out of memory\n");
		return;
	}

	for(i = 0; i < row; i++) {
		(*raw)[i] = malloc(col * sizeof(unsigned int));
		if((*raw)[i] == 0){
			logIt(ERROR, "out of memory in inner loop");
			fprintf(stderr, "out of memory\n");
			return;
		}
	}

	logIt(DEBUG, "allocateUnsignedIntArray(unsigned int ***raw, int row, int col) finished.");
}

void freeUnsignedIntArray(unsigned int ***raw, int row, int col) {
	logIt(DEBUG, "freeUnsignedIntArray(unsigned int ***raw, int row, int col) started.");
	int i = 0;
	for(i = 0; i < row; i++) {
		free((*raw)[i]);
	}
	free(*raw);
	logIt(DEBUG, "freeUnsignedIntArray(unsigned int ***raw, int row, int col) finished.");
}



int loadPGMToRaw(unsigned int ***raw, FILE *data){
	logIt(DEBUG, "loadPGMToRaw(unsigned int ***raw, FILE *data) started.");
	int return_value = 0;
	int i = 0;
	int j = 0;
	char str[200];
	int noOneCares = 0;


	logIt(TRACE, "here??");
	//Read P2
	fgets(str, 200, data);
	if(!(str[0] == 'P' && str[1] == '2')){
		logIt(ERROR, "Not a pgm.");
		return 1;
	}


	fgets(str, 200, data); //Hopefully a commentary
	fscanf(data,"%d",&noOneCares);
	fscanf(data,"%d",&noOneCares);
	fgets(str, 200, data);

	fgets(str, 200, data);//colordepth, we dont care about



	for(i=0; i < ROWS; i++){
		for(j = 0; j<COLS; j++){
			fscanf(data,"%d",&((*raw)[i][j]));
		}

	}


	logIt(DEBUG, "loadPGMToRaw(unsigned int ***raw, FILE *data) finished.");
	return return_value;
}

int project(int angle){
	logIt(DEBUG, "project(int angle) started.");
	int t = 0;
	int x = 0;
	int y = 0;
	int count = angle;
	angle = angle - NUM_ANGLES/2;
	int s = 0;
	int mat = 0;
	double energy = MINENERGY;
	double alpha = (((double)(angle))/((double)NUM_ANGLES))*(PI);

	for(s = -SINOGRAMSIZE/2; s<SINOGRAMSIZE/2; s++){
		for(t = -COLS; t<COLS; t++){
			x = (int)(t*sin(alpha)+s*cos(alpha)+0.5+COLS/2);
			y = (int)(-t*cos(alpha)+s*sin(alpha)+0.5+COLS/2);
			if(x>=0 && x<COLS && y>=0 && y <COLS){
				for(energy = MINENERGY; energy<=MAXENERGY; energy+=((MAXENERGY-MINENERGY)/ENERGYLEVELS)){
					for(mat = MINMAT; mat< MAXMAT; mat++){
						result[count][s+SINOGRAMSIZE/2] += getAttenuation(mat, energy, x,y);
					}
				}
			}
		}
	}
	logIt(DEBUG, "project(int angle) finished.");
	return 0;
}

int exportPGM(FILE* out, unsigned int** write, int x, int y){
	logIt(DEBUG, "exportPGM(FILE* out, unsigned int** write, int x, int y) started.");
	int i = 0;
	int j = 0;

	int min = write[0][0];
	int max = write[0][0];


	//Output as a picture file
	logIt(TRACE, "Output as a picture file");
	for(i = 0; i<x;i++){
		for(j = 0; j<y; j++){

			//printf("i:%d,j:%d\n",i,j);
			//fflush(stdout);
			if(write[i][j]<min){
				logIt(TRACE, "New min found");
				min = write[i][j];
				//printf("newmin\n");
			}

			if(write[i][j]>max){
				logIt(TRACE, "New max found");
				max = write[i][j];
				//printf("newmax:%d\n", max);
			}

		}
	}



	//FILE *res = fopen("result.pgm", "wb");
	fprintf(out, "P2\n# Created by Sim\n%d %d\n255\n", y, x);
	//fprintf(out, "P2\n# Created by Sim\n%d %d\n255\n", y, x);

	for(i = 0; i<x;i++){
		for(j = 0; j<y; j++){
			//printf("%d \n",(int)(((((double)write[i][j])-min)/((double)max-(double)min))*255.0));
			//fflush(stdout);
			fprintf(out,"%d ",(int)(((((double)write[i][j])-min)/((double)max-(double)min))*255.0));
		}
	}

	fclose(out);
	logIt(DEBUG, "exportPGM(FILE* out, unsigned int** write, int x, int y) finished.");
	return 0;
}



void setUpRawFiles(char *pathToSlices){
	logIt(DEBUG, "setUpRawFiles(char *pathToSlices) started.");
	char paths[256];

	strcpy(paths, pathToSlices);
	strcat(paths, "/air.pgm");
	logIt(TRACE, "Path to air: %s", paths);
	airImage = fopen(paths,"r");

	strcpy(paths, pathToSlices);
	strcat(paths, "/bone.pgm");
	logIt(TRACE, "Path to bone: %s", paths);
	boneImage = fopen(paths,"r");

	strcpy(paths, pathToSlices);
	strcat(paths, "/iron.pgm");
	logIt(TRACE, "Path to iron: %s", paths);
	ironImage = fopen(paths,"r");

	strcpy(paths, pathToSlices);
	strcat(paths, "/muscle.pgm");
	logIt(TRACE, "Path to muscle: %s", paths);
	muscleImage = fopen(paths,"r");

	strcpy(paths, pathToSlices);
	strcat(paths, "/tissue.pgm");
	logIt(TRACE, "Path to tissue: %s", paths);
	tissueImage = fopen(paths,"r");

	strcpy(paths, pathToSlices);
	strcat(paths, "/water.pgm");
	logIt(TRACE, "Path to water: %s", paths);
	waterImage = fopen(paths,"r");

	logIt(DEBUG, "setUpRawFiles(char *pathToSlices) finished.");
}

void allocateAllRaws() {
	logIt(DEBUG, "allocateAllRaws()  started.");
	(void) allocateUnsignedIntArray(&ironRaw, ROWS, COLS);
	(void) allocateUnsignedIntArray(&boneRaw, ROWS, COLS);
	(void) allocateUnsignedIntArray(&waterRaw, ROWS, COLS);
	(void) allocateUnsignedIntArray(&airRaw, ROWS, COLS);
	(void) allocateUnsignedIntArray(&muscleRaw, ROWS, COLS);
	(void) allocateUnsignedIntArray(&tissueRaw, ROWS, COLS);
	logIt(DEBUG, "allocateAllRaws()  finished.");
}

void freeAllRaws() {
	logIt(DEBUG, "freeAllRaws()  started.");
	freeUnsignedIntArray(&ironRaw, ROWS, COLS);
	freeUnsignedIntArray(&boneRaw, ROWS, COLS);
	freeUnsignedIntArray(&waterRaw, ROWS, COLS);
	freeUnsignedIntArray(&airRaw, ROWS, COLS);
	freeUnsignedIntArray(&muscleRaw, ROWS, COLS);
	freeUnsignedIntArray(&tissueRaw, ROWS, COLS);
	logIt(DEBUG, "freeAllRaws()  finished.");
}

void closeAllInputImages() {
	logIt(DEBUG, "closeAllInputImages()  started.");
	fclose(airImage);
	fclose(boneImage);
	fclose(ironImage);
	fclose(muscleImage);
	fclose(tissueImage);
	fclose(waterImage);
	logIt(DEBUG, "closeAllInputImages()  finished.");
}

