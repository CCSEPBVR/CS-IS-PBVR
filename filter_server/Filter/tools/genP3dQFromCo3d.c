// This script is used to transform the co3d.dat example file 
// from the AVS fileformat to the p3d file format (only xys file)
// This script has fixed the dimensions to { 220, 220, 48 } 
// These must be changed when transforming other files

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

int main() {
    const char *pd3dFile = "pd3d.dat";
    const char *psiFile = "psid.dat";

    printf("Programmstart\n");
    struct stat pd3dInfo;
    struct stat psiInfo;
    if (stat(pd3dFile, &pd3dInfo) != 0) {
        printf("ERROR reading file states for file: %s \n", pd3dFile);
        return 0;
    }

    if (stat(psiFile, &psiInfo) != 0) {
        printf("ERROR reading file states for file: %s \n", psiFile);
        return 0;
    }

    printf("FILE SIZE of %s : %lu\n", pd3dFile, pd3dInfo.st_size);
    printf("FILE SIZE of %s : %lu\n", psiFile, psiInfo.st_size); // psid.dat file is smaller than the pd3d file

    unsigned int totalCoordinates = psiInfo.st_size / sizeof (float); // use psid.dat file size
    printf("Number of total coordinates in file: %d\n", totalCoordinates);
    printf("Sizeof(float): %lu\n", sizeof (float));

    float *p3dContent = malloc(totalCoordinates * sizeof (float));
    float *psiContent = malloc(totalCoordinates * sizeof (float));

    if (p3dContent == NULL || psiContent == NULL) {
        printf("Not enough memory");
        return 0;
    }


    FILE *pd3d_fh = fopen(pd3dFile, "rb");
    FILE *psi_fh = fopen(psiFile, "rb");
    if (pd3d_fh == NULL || psi_fh == NULL) {
        printf("File could not be opened");
        return 0;
    }



    size_t ret = fread(psiContent, sizeof (float), totalCoordinates, psi_fh);
    fclose(psi_fh);
    printf("FREAD psi: %lu\n", ret);


    // WRITE PART
    int step;
    char outname[64] ;
    for (step = 1; step < 6; step++) {

        ret = fread(p3dContent, sizeof (float), totalCoordinates, pd3d_fh);
        printf("FREAD p3d: %lu\n", ret);
        unsigned int stride = 0;
        int i;
        sprintf(outname, "Co3d-bin%06d.f", step);
        FILE *outputfile = fopen(outname, "wb");
        int numberofgrids = 1;
        int dimensions[] = {220, 220, 48, 2};
        fwrite(&numberofgrids, sizeof (int), 1, outputfile);
        fwrite(dimensions, sizeof (int), 4, outputfile);
        // i = 1 means skip for byte
        for (i = 1; i < totalCoordinates; i++) {
            fwrite(&p3dContent[i], sizeof (float), 1, outputfile);
            //printf("Var: %f \n", content[i]);
        }

        for (i = 1; i < totalCoordinates; i++) {
            fwrite(&psiContent[i], sizeof (float), 1, outputfile);
            //printf("Var: %f \n", content[i]);
        }

        fclose(outputfile);
    }
    fclose(pd3d_fh);
    free(p3dContent);
    free(psiContent);
    return 0;
}
