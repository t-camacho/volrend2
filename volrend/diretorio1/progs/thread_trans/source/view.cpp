#include "incl.h"

#define XAXIS		1
#define YAXIS		2
#define ZAXIS		3

float transformation_matrix[4][4];
float out_invvertex[2][2][2][NM];
float uout_invvertex[2][2][2][NM];

long frust_len;

long image_zlen;

float out_diag_len[NM];
float out_diag_length;

float depth_cueing[MAX_OUTLEN];

float in_max[NM];

long opc_xlen, opc_xylen;
long norm_xlen, norm_xylen;

float invmatrix[4][4];

void Compute_Pre_View() {
    for (long i = 0; i < NM; i++) {
        out_diag_len[i] = opc_len[i] - 1;
    }
    out_diag_length = out_diag_len[X]*out_diag_len[X] + out_diag_len[Y]*out_diag_len[Y] +
                      out_diag_len[Z]*out_diag_len[Z];

    out_diag_length = sqrt(out_diag_length);
    frust_len = NINT(out_diag_length) + 1;
    image_zlen = frust_len - 1;

    if (image_zlen > MAX_OUTLEN)
        std::cout << "MAX_OUTLEN exceeded in Ray_Trace.\n";

    for (long outz = 0; outz <= image_zlen; outz++) {
        depth_cueing[outz] = depth_hither - 
                             pow((float)(outz)/(float)image_zlen,depth_exponent) *
                             (depth_hither - depth_yon);
        depth_cueing[outz] = MIN(MAX(depth_cueing[outz],0.0),1.0);
    }

    in_max[X] = (float)(opc_len[X]-1)-SMALL-1.0/(float)MAX_PIXEL;
    in_max[Y] = (float)(opc_len[Y]-1)-SMALL-1.0/(float)MAX_PIXEL;
    in_max[Z] = (float)(opc_len[Z]-1)-SMALL-1.0/(float)MAX_PIXEL;

    opc_xlen = opc_len[X] + 1;
    opc_xylen = opc_len[X] * opc_len[Y] + 1;

    norm_xlen = norm_len[X] + 1;
    norm_xylen = norm_len[X] * norm_len[Y] + 1;
}

void Transform_Point(float xold, float yold, float zold, 
                     float *xnew, float *ynew, float *znew) {
	*xnew =
	    xold * transformation_matrix[0][0] +
            yold * transformation_matrix[1][0] +
                zold * transformation_matrix[2][0] +
                    transformation_matrix[3][0];

	*ynew =
	    xold * transformation_matrix[0][1] +
            yold * transformation_matrix[1][1] +
                zold * transformation_matrix[2][1] +
                    transformation_matrix[3][1];

	*znew =
	    xold * transformation_matrix[0][2] +
            yold * transformation_matrix[1][2] +
                zold * transformation_matrix[2][2] +
                    transformation_matrix[3][2];
}

void Select_View(float delta_angle, int axis) {
    Load_Identity_Matrix(invmatrix);

    Inverse_Concatenate_Translation(invmatrix,
                                    (float)(out_diag_len[X])/2.0,
                                    (float)(out_diag_len[Y])/2.0,
                                    (float)(out_diag_len[Z])/2.0);

    Inverse_Concatenate_Scaling(invmatrix,1.0,1.0,1.0/(float)ZSCALE);

    if (frame != 0)
        angle[axis] = angle[axis] + delta_angle;

    Inverse_Concatenate_Rotation(invmatrix,XAXIS,-angle[X]);
    Inverse_Concatenate_Rotation(invmatrix,YAXIS,-angle[Y]);

    Inverse_Concatenate_Rotation(invmatrix,XAXIS,30.0);

    Inverse_Concatenate_Translation(invmatrix,
                                    -out_diag_length/2.0,
                                    -out_diag_length/2.0,
                                    -out_diag_length/2.0);

    Load_Transformation_Matrix(invmatrix);
    Compute_Input_Dimensions();
    Compute_Input_Unit_Vector();
}

void Load_Identity_Matrix(float matrix[4][4]) {
	for (long i = 0; i < 4; i++) {
		for (long j = 0; j < 4; j++) {
			matrix[i][j] = 0;
		}
		matrix[i][i] = 1.0;
	}
}

void Inverse_Concatenate_Translation(float matrix[4][4],
                                     float xoffset, 
                                     float yoffset, 
                                     float zoffset) {
	float translation_matrix[4][4];
	Load_Translation_Matrix(translation_matrix,xoffset,yoffset,zoffset);
	Inverse_Concatenate_Transform(matrix,translation_matrix);
}

void Load_Translation_Matrix(float matrix[4][4],
                             float xoffset,
                             float yoffset,
                             float zoffset) {
	Load_Identity_Matrix(matrix);
	matrix[3][0] = xoffset;
	matrix[3][1] = yoffset;
	matrix[3][2] = zoffset;
}

void Inverse_Concatenate_Transform(float composite_matrix[][4],
                                   float transformation_matrix[][4]) {
	float temp_matrix[4][4];
	Multiply_Matrices(transformation_matrix,composite_matrix,temp_matrix);
	Copy_Matrix(temp_matrix,composite_matrix);
}

void Multiply_Matrices(float input_matrix1[][4],
                       float input_matrix2[][4],
                       float output_matrix[][4]) {
	for (long i = 0; i < 4; i++) {
		for (long j = 0; j < 4; j++) {
			output_matrix[i][j] =
			    input_matrix1[i][0] *
			    input_matrix2[0][j] +
			    input_matrix1[i][1] *
			    input_matrix2[1][j] +
			    input_matrix1[i][2] *
			    input_matrix2[2][j] +
			    input_matrix1[i][3] *
			    input_matrix2[3][j];
		}
	}
}


void Copy_Matrix(float input_matrix[][4],
                 float output_matrix[][4]) {
	for (long i = 0; i < 4; i++) {
		for (long j = 0; j < 4; j++) {
			output_matrix[i][j] = input_matrix[i][j];
		}
	}
}

void Inverse_Concatenate_Scaling(float matrix[4][4],
                                 float xscale,
                                 float yscale,
                                 float zscale) {
	float scaling_matrix[4][4];
	Load_Scaling_Matrix(scaling_matrix,xscale,yscale,zscale);
	Inverse_Concatenate_Transform(matrix,scaling_matrix);
}

void Load_Scaling_Matrix(float matrix[4][4],
                         float xscale,
                         float yscale,
                         float zscale) {
	Load_Identity_Matrix(matrix);
	matrix[0][0] = xscale;
	matrix[1][1] = yscale;
	matrix[2][2] = zscale;
}

void Inverse_Concatenate_Rotation(float matrix[4][4], float angle, long axis) {
	float rotation_matrix[4][4];
	Load_Rotation_Matrix(rotation_matrix,axis,angle);
	Inverse_Concatenate_Transform(matrix,rotation_matrix);
}

void Load_Rotation_Matrix(float matrix[4][4], float angle, long axis) {
	Load_Identity_Matrix(matrix);
	if (axis == XAXIS) {
		matrix[1][1] = cos(angle/180.0*PI);
		matrix[1][2] = sin(angle/180.0*PI);
		matrix[2][1] = -sin(angle/180.0*PI);
		matrix[2][2] = cos(angle/180.0*PI);
	}
	else if (axis == YAXIS) {
		matrix[0][0] = cos(angle/180.0*PI);
		matrix[0][2] = -sin(angle/180.0*PI);
		matrix[2][0] = sin(angle/180.0*PI);
		matrix[2][2] = cos(angle/180.0*PI);
	}
	else if (axis == ZAXIS) {
		matrix[0][0] = cos(angle/180.0*PI);
		matrix[0][1] = sin(angle/180.0*PI);
		matrix[1][0] = -sin(angle/180.0*PI);
		matrix[1][1] = cos(angle/180.0*PI);
	}
}

void Load_Transformation_Matrix(float matrix[4][4]) {
	Copy_Matrix(matrix,transformation_matrix);
}

void Compute_Input_Dimensions() {
    float in_invvertex[2][2][2][NM];
    
    in_invvertex[0][0][0][X] = 0;
    in_invvertex[0][0][0][Y] = 0;
    in_invvertex[0][0][0][Z] = 0;
    
    in_invvertex[0][0][1][X] = frust_len-1;
    in_invvertex[0][0][1][Y] = 0;
    in_invvertex[0][0][1][Z] = 0;
    
    in_invvertex[0][1][0][X] = 0;
    in_invvertex[0][1][0][Y] = frust_len-1;
    in_invvertex[0][1][0][Z] = 0;
    
    in_invvertex[0][1][1][X] = frust_len-1;
    in_invvertex[0][1][1][Y] = frust_len-1;
    in_invvertex[0][1][1][Z] = 0;
    
    in_invvertex[1][0][0][X] = 0;
    in_invvertex[1][0][0][Y] = 0;
    in_invvertex[1][0][0][Z] = frust_len-1;
    
    in_invvertex[1][0][1][X] = frust_len-1;
    in_invvertex[1][0][1][Y] = 0;
    in_invvertex[1][0][1][Z] = frust_len-1;
    
    in_invvertex[1][1][0][X] = 0;
    in_invvertex[1][1][0][Y] = frust_len-1;
    in_invvertex[1][1][0][Z] = frust_len-1;
    
    in_invvertex[1][1][1][X] = frust_len-1;
    in_invvertex[1][1][1][Y] = frust_len-1;
    in_invvertex[1][1][1][Z] = frust_len-1;
    
    for (long z = 0; z < 2; z++) {
        for (long y = 0; y < 2; y++) {
            for (long x = 0; x < 2; x++) {
                Transform_Point(in_invvertex[z][y][x][X],
                                in_invvertex[z][y][x][Y],
                                in_invvertex[z][y][x][Z],
                                &out_invvertex[z][y][x][X],
                                &out_invvertex[z][y][x][Y],
                                &out_invvertex[z][y][x][Z]);
            }
        }
    }
}

void Compute_Input_Unit_Vector() {
    float uin_invvertex[2][2][2][NM];    
    
    uin_invvertex[0][0][0][X] = 0;
    uin_invvertex[0][0][0][Y] = 0;
    uin_invvertex[0][0][0][Z] = 0;
    
    uin_invvertex[0][0][1][X] = 1.0;
    uin_invvertex[0][0][1][Y] = 0;
    uin_invvertex[0][0][1][Z] = 0;
    
    uin_invvertex[0][1][0][X] = 0;
    uin_invvertex[0][1][0][Y] = 1.0;
    uin_invvertex[0][1][0][Z] = 0;
    
    uin_invvertex[0][1][1][X] = 1.0;
    uin_invvertex[0][1][1][Y] = 1.0;
    uin_invvertex[0][1][1][Z] = 0;
    
    uin_invvertex[1][0][0][X] = 0;
    uin_invvertex[1][0][0][Y] = 0;
    uin_invvertex[1][0][0][Z] = 1.0;
    
    uin_invvertex[1][0][1][X] = 1.0;
    uin_invvertex[1][0][1][Y] = 0;
    uin_invvertex[1][0][1][Z] = 1.0;
    
    uin_invvertex[1][1][0][X] = 0;
    uin_invvertex[1][1][0][Y] = 1.0;
    uin_invvertex[1][1][0][Z] = 1.0;
    
    uin_invvertex[1][1][1][X] = 1.0;
    uin_invvertex[1][1][1][Y] = 1.0;
    uin_invvertex[1][1][1][Z] = 1.0;
    
    for (long z = 0; z < 2; z++) {
        for (long y = 0; y < 2; y++) {
            for (long x = 0; x < 2; x++) {
                Transform_Point(uin_invvertex[z][y][x][X],
                                uin_invvertex[z][y][x][Y],
                                uin_invvertex[z][y][x][Z],
                                &uout_invvertex[z][y][x][X],
                                &uout_invvertex[z][y][x][Y],
                                &uout_invvertex[z][y][x][Z]);
            }
        }
    }
}