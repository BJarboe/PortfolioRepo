#include <stdio.h>
#include <stdlib.h>

// Structure to hold grayscale pixel data
typedef struct {
    unsigned char value;
} PixelGray;

// Function to read a PGM image into a 2D array
PixelGray** readPGM(const char* filename, int* width, int* height); 

// Function to write a 2D matrix as a PGM image
void writePGM(const char* filename, PixelGray** matrix, int* width, int* height);

// Function to threshold the image matrix
PixelGray** threshold(PixelGray** matrix, int* width, int* height);

// Function to rotate the image matrix
PixelGray** rotate(PixelGray** matrix, int* width, int* height);


//main function - DO NOT MODIFY
int main() {
    int width, height;  // variable to hold width and height. Use reference in other functions

    PixelGray** image_original = readPGM("lenna.pgm", &width, &height);
    // Now you have the grayscale image data in the 'image_original' 2D array

    // Access pixel data using image[row][col].value
    // For example, to access the pixel at row=2, col=3:
    // unsigned char pixel_value = image[2][3].value;
    // writePGM("copy.pgm", image_original, &width, &height);
    // Create a new 2D array 'image_thresh' to store the threshold image
    PixelGray** image_thresh = threshold(image_original, &width, &height);
    //write the image data as "threshold.pgm"
    writePGM("threshold.pgm", image_thresh, &width, &height);

    // Create a new 2D array 'image_rotate' to store the rotated image
    PixelGray** image_rotate = rotate(image_original, &width, &height);
    //write the image data as "rotate.pgm"
    writePGM("rotate.pgm", image_rotate, &width, &height);

    image_rotate = rotate(image_rotate, &width, &height);
    //write the image data as "rotate_again.pgm"
    writePGM("rotate_again.pgm", image_rotate, &width, &height);

    // Free the allocated memory when you're done
    for (int i = 0; i < height; ++i) {
        free(image_original[i]);
        free(image_thresh[i]);
        free(image_rotate[i]);
    }
    free(image_original);
    free(image_thresh);
    free(image_rotate);
    return 0;
}


void writePGM(const char* filename, PixelGray** matrix, int* width, int* height){
    int dim = *width * *height;
    FILE * newFile = fopen(filename, "wb");
    fprintf(newFile, "P5\n%d %d\n255\n", *width, *height);
    for (int i = 0; i < *height; i++){
        for (int j = 0; j < *width; j++){
            fwrite(&(matrix[i][j].value), sizeof(PixelGray), 1, newFile);
        }
    }
    fclose(newFile);
}

PixelGray** readPGM(const char* filename, int* width, int* height){

   
   char skip[3]; // char for unused fscanf statements
   int maxValue; // placeholder

   // Open File, abort if unsuccessful
   FILE * image = fopen(filename, "rb");
   if (image == NULL){
    printf("Unable to open file\n");
    exit(1);
   }

    // READ PGM
    fscanf(image, "%2s", skip);
    fscanf(image, "%d %d", width, height);
    printf("%d %d\n", *width, *height);
    fscanf(image, "%d", &maxValue);
    
    
    // Allocate memory for an array of pointers to *height
    PixelGray **matrix = (PixelGray **)malloc(*height * sizeof(PixelGray *));
    if (matrix == NULL) {
        printf("Error: Unable to allocate memory for *height\n");
        exit(1);
    }
    // Allocate memory for each row (array of integers)
    for (int i = 0; i < *height; i++) {
        matrix[i] = (PixelGray *)malloc(*width * sizeof(PixelGray));
        if (matrix[i] == NULL) {
            printf("Error: Unable to allocate memory for columns in row %d\n", i);
            exit(1);
        }
    }

    fgetc(image);

    // Initialize and use the 2D array
    for (int i = 0; i < *height; i++) {
        for (int j = 0; j < *width; j++) {
            fread(&matrix[i][j].value, sizeof(unsigned char), 1, image);
            // printf("%d ", matrix[i][j].value);
        }
        // printf("\n");
    }
    
    // printf("width: %d  height %d", *width, *height);

   // Close File
   fclose(image);
   
   return matrix;


}


PixelGray** threshold(PixelGray** matrix, int* width, int* height){

    //Allocate new matrixC, matrix with high contrast
    PixelGray **matrixC = (PixelGray **)malloc(*height * sizeof(PixelGray *));
    if (matrixC == NULL) {
        printf("Error: Unable to allocate memory for *height\n");
        exit(1);
    }

    // Allocate memory for each row (array of integers)
    for (int i = 0; i < *height; i++) {
        matrixC[i] = (PixelGray *)malloc(*width * sizeof(PixelGray));
        if (matrix[i] == NULL) {
            printf("Error: Unable to allocate memory for columns in row %d\n", i);
            exit(1);
        }
    }

    // Initialize the 2D array checking if it is above or below the threshold and min/maxxing accordingly
    for (int i = 0; i < *height; i++) {
        for (int j = 0; j < *width; j++) {
            if (matrix[i][j].value > 80){
                matrixC[i][j].value = 255;
            }
            else{
                matrixC[i][j].value = 0;
            }
        }
    }

    return matrixC;

}


PixelGray** rotate(PixelGray** matrix, int* width, int* height){
    

    // Allocate memory for a new transposed matrixT

    // Allocate memory for an array of pointers to *width
    PixelGray **matrixT = (PixelGray **)malloc(*width * sizeof(PixelGray *));
    if (matrixT == NULL) {
        printf("Error: Unable to allocate memory for *width\n");
        exit(1);
    }
    // Allocate memory for each row (array of integers)
    for (int i = 0; i < *width; i++) {
        matrixT[i] = (PixelGray *)malloc(*height * sizeof(PixelGray));
        if (matrixT[i] == NULL) {
            printf("Error: Unable to allocate memory for columns in row %d\n", i);
            exit(1);
        }
    }
    // Initialize matrix with the swapped indices
    for (int i = 0; i < *height; i++) {
        for (int j = 0; j < *width; j++) {
            matrixT[j][i].value = matrix[i][j].value;
        }
    }

    return matrixT;
    
}
