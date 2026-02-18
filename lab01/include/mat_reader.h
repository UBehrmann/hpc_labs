/**
 * @authors
 * - Bruno Carvalho
 *
 * @file    mat_reader.h
 * @brief   Interface du module de lecture de fichiers MAT (MATLAB) pour données ECG.
 * @details Ce module fournit les fonctions et variables nécessaires pour
 *          charger des données ECG depuis un fichier .mat et les stocker
 *          dans des structures globales exploitables par le reste du programme.
 *
 * Dépendance: libmatio (matio.h)
 */

#ifndef MAT_READER_H
#define MAT_READER_H

#include "output_structs.h"

extern double ecg_data[LEADS][MAX_SAMPLES];
extern int sample_count;


int read_mat(const char *filename);

#endif /* MAT_READER_H */


/*To include in MAIN :
#define MAT_READER_IMPLEMENTATION
#include "mat_reader.h"
*/

#ifdef MAT_READER_IMPLEMENTATION

#include <matio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int mr_is_supported_class(enum matio_classes c)
{
    return (c == MAT_C_DOUBLE || c == MAT_C_SINGLE ||
            c == MAT_C_INT16  || c == MAT_C_UINT16 ||
            c == MAT_C_INT32  || c == MAT_C_UINT32 ||
            c == MAT_C_INT8   || c == MAT_C_UINT8  ||
            c == MAT_C_INT64  || c == MAT_C_UINT64);
}

static double mr_get_as_double(const void *data, size_t idx, enum matio_classes c)
{
    switch (c) {
        case MAT_C_DOUBLE: return ((const double*)data)[idx];
        case MAT_C_SINGLE: return (double)((const float*)data)[idx];

        case MAT_C_INT8:   return (double)((const int8_t*)data)[idx];
        case MAT_C_UINT8:  return (double)((const uint8_t*)data)[idx];

        case MAT_C_INT16:  return (double)((const int16_t*)data)[idx];
        case MAT_C_UINT16: return (double)((const uint16_t*)data)[idx];

        case MAT_C_INT32:  return (double)((const int32_t*)data)[idx];
        case MAT_C_UINT32: return (double)((const uint32_t*)data)[idx];

        case MAT_C_INT64:  return (double)((const int64_t*)data)[idx];
        case MAT_C_UINT64: return (double)((const uint64_t*)data)[idx];

        default: return 0.0; // non supporté
    }
}

int read_mat(const char *filename)
{
    sample_count = 0;
    for (int l = 0; l < LEADS; l++)
        for (int s = 0; s < MAX_SAMPLES; s++)
            ecg_data[l][s] = 0.0;

    mat_t *matfp = Mat_Open(filename, MAT_ACC_RDONLY);
    if (!matfp) {
        perror("Mat_Open");
        return -1;
    }

    matvar_t *val = Mat_VarRead(matfp, "val");
    if (!val) {
        fprintf(stderr, "Erreur: variable 'val' introuvable dans %s\n", filename);
        Mat_Close(matfp);
        return -2;
    }

    if (val->rank != 2) {
        fprintf(stderr, "Erreur: 'val' n'est pas une matrice 2D (rank=%d)\n", val->rank);
        Mat_VarFree(val);
        Mat_Close(matfp);
        return -3;
    }

    if (!mr_is_supported_class(val->class_type)) {
        fprintf(stderr, "Erreur: type de 'val' non supporté (class_type=%d)\n", (int)val->class_type);
        Mat_VarFree(val);
        Mat_Close(matfp);
        return -4;
    }

    size_t d0 = val->dims[0];
    size_t d1 = val->dims[1];

    int orientation = 0; 
    size_t N = 0;

    if (d0 == (size_t)LEADS) {
        orientation = 1;
        N = d1;
    } else if (d1 == (size_t)LEADS) {
        orientation = 2;
        N = d0;
    } else {
        fprintf(stderr,
                "Erreur: dimensions inattendues pour 'val' : %zu x %zu (LEADS=%d)\n",
                d0, d1, LEADS);
        Mat_VarFree(val);
        Mat_Close(matfp);
        return -5;
    }

    if (N > (size_t)MAX_SAMPLES) N = (size_t)MAX_SAMPLES;

    const size_t rows = d0;
    const size_t cols = d1;

    const void *data = val->data;
    if (!data) {
        fprintf(stderr, "Erreur: 'val->data' est NULL (variable non chargée?)\n");
        Mat_VarFree(val);
        Mat_Close(matfp);
        return -6;
    }

    if (orientation == 1) {
        for (int lead = 0; lead < LEADS; lead++) {
            for (size_t s = 0; s < N; s++) {
                size_t idx = (size_t)lead + s * rows;
                ecg_data[lead][(int)s] = mr_get_as_double(data, idx, val->class_type);
            }
        }
    } else {
        for (int lead = 0; lead < LEADS; lead++) {
            for (size_t s = 0; s < N; s++) {
                size_t idx = s + (size_t)lead * rows;
                ecg_data[lead][(int)s] = mr_get_as_double(data, idx, val->class_type);
            }
        }
    }

    sample_count = (int)N;

    Mat_VarFree(val);
    Mat_Close(matfp);

    printf(".mat chargé avec %d leads et %d échantillons.\n", LEADS, sample_count);
    return 0;
}

#endif /* MAT_READER_IMPLEMENTATION */
