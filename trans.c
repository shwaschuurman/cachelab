/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N]){
	//used to iterate through blocks of data, and then through those blocks respectively 
	int cBlock, rBlock, c, r;
	//temp variables for transposing on diagonals
	int temp1, temp2;

	//test case 1
	if(N == 32 && M == 32){
		//iterate through "blocks" of the arrays using a block size of 8 via columns first
		for(cBlock = 0; cBlock < N; cBlock += 8){
			for(rBlock = 0; rBlock < N; rBlock += 8){
				//iterate through indiviual elements in the blocks
				for(r = rBlock; r < rBlock + 8; r++){
					for(c = cBlock; c < cBlock + 8; c++){
						//if on a diagonal, store the value and the row/column in temps
						if(r == c){
							temp1 = A[r][c];
							temp2 = r;
						}
						//if not on a diagonal, transpose
						else{
							B[c][r] = A[r][c];
						}
					}

					//transpose the diagonal
					if(rBlock == cBlock){
						B[temp2][temp2] = temp1;	
					}
				}
			}
		}
	}
	//test case 2
	else if(N == 64 && M == 64){
		//iterate through blocks normally using a block size of 4
		for(r = 0; r < N; r += 4){
			for(c = 0; c < N; c += 4){
				//transpose elements from column c+{3,2,1,0} in A to row c+{3,2,1,0} in B. Not good by any means but under 2000
				B[c+3][r] = A[r][c+3];
				B[c+3][r+1] = A[r+1][c+3];
				B[c+3][r+2] = A[r+2][c+3];
				B[c+2][r] = A[r][c+2];
				B[c+2][r+1] = A[r+1][c+2];
				B[c+2][r+2] = A[r+2][c+2];;
				B[c+1][r] = A[r][c+1];
				B[c+1][r+1] = A[r+1][c+1];
				B[c+1][r+2] = A[r+2][c+1];
				B[c][r] = A[r][c];
				B[c][r+1] = A[r+1][c];
				B[c][r+2] = A[r+2][c];
				B[c][r+3] = A[r+3][c];
				B[c+1][r+3] = A[r+3][c+1];
				B[c+2][r+3] = A[r+3][c+2];
				B[c+3][r+3] = A[r+3][c+3];
			}
		}
	}

	//test case 3 and all others
	else{
		//Iterate the same way as test case 1, but with block size 16
		for(cBlock = 0; cBlock < M; cBlock += 16){
			for(rBlock = 0; rBlock < N; rBlock += 16){
				//once again very similar to test case 1, except making sure we don't fall off of the matrix
				for(r = rBlock; (r < rBlock + 16) && (r < N); r++){
					for(c = cBlock; (c < cBlock + 16) && (c < M); c++){
						//if on a diagonal, store the value and the row/column in temps
						if(r == c){
							temp1 = A[r][c];
							temp2 = r;
						}
						//if not on a diagonal, transpose
						else{
							B[c][r] = A[r][c];
						}
					}

					//transpose the diagonal
					if(rBlock == cBlock){
						B[temp2][temp2] = temp1;
					}
				}
			}
		}
	}
}
						

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    

}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc); 

}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

