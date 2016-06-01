#include<stdio.h>
#include<math.h>
#include<stdlib.h>
#include "sw_config_register.h"
#include "macro_select.h"

sw_config_register * sw_crgs = NULL;

void sw_confg_regiser_create(int swnum)
{
    sw_crgs = (sw_config_register*)malloc(swnum * sizeof(sw_config_register));
    if(sw_crgs == NULL)
    {
	printf("sw_crgs alloc error!\n");
	exit(0);
    }
}

#ifdef NETWORK_TYPE_FTREE
void sw_confg_register_init(sw_config_register * rgs, int index,int *dms)
{
}
#endif

#ifdef NETWORK_TYPE_TORFT
void sw_confg_register_init(sw_config_register * rgs, int index,int *dms)
{
}
#endif

#ifdef NETWORK_TYPE_VTORFT
void sw_confg_register_init(sw_config_register * rgs, int index,int *dms)
{
}
#endif

#ifdef NETWORK_TYPE_D7K
#define A 0
#define B 1
#define C 2
#define D 3
#define E 4

//typedef enum {A=0, B, C, D, E} drct;

void sw_confg_register_init(sw_config_register * rgs, int index,int *dms)
{
    int i;
    rgs->inner_id = index % 4;
    i = index / 4;
    rgs->x_id[A] = i % dms[0];
    rgs->x_id[B] = (i / dms[0]) % dms[1];
    rgs->x_id[C] = i / (dms[0]*dms[1]) % dms[2];
    rgs->x_id[D] = i / (dms[0]*dms[1]*dms[2]) % dms[3] ;
    rgs->x_id[E] = rgs->inner_id;
    //printf("A = %d, B = %d, C = %d, D = %d \n",rgs->x_id[A],rgs->x_id[B],rgs->x_id[C],rgs->x_id[D]);

    for(i = 0; i < 4; i++)
    {
	if(rgs->x_id[i] == 0)
	    rgs->dataline[i] = true;
	else 
	    rgs->dataline[i] = false;
	
	rgs->esp_vc[i] = 0;
    }
    if( rgs->inner_id == 0)
    {
	rgs->xp_pd[0] = 3;
	rgs->xp_pd[1] = 0;
	rgs->xp_pd[2] = 1;
	rgs->xp_pd[3] = 2;

	rgs->xn_pd[0] = 4;
	rgs->xn_pd[1] = 0;
	rgs->xn_pd[2] = 1;
	rgs->xn_pd[3] = 2;
	
	rgs->esp_vc[E] = 0;
	rgs->xp_pd[E]  = 1;
	rgs->xn_pd[E]  = 2;
    }
    if( rgs->inner_id == 1)
    {
	rgs->xp_pd[0] = 0;
	rgs->xp_pd[1] = 3;
	rgs->xp_pd[2] = 1;
	rgs->xp_pd[3] = 2;
	
	rgs->xn_pd[0] = 0;
	rgs->xn_pd[1] = 4;
	rgs->xn_pd[2] = 1;
	rgs->xn_pd[3] = 2;

	rgs->esp_vc[E] = 0;
	rgs->xp_pd[E]  = 1;
	rgs->xn_pd[E]  = 2;
    }
    if( rgs->inner_id == 2)
    {
	rgs->xp_pd[0] = 0;
	rgs->xp_pd[1] = 1;
	rgs->xp_pd[2] = 3;
	rgs->xp_pd[3] = 2;

	rgs->xn_pd[0] = 0;
	rgs->xn_pd[1] = 1;
	rgs->xn_pd[2] = 4;
	rgs->xn_pd[3] = 2;

	rgs->esp_vc[E] = 0;
	rgs->xp_pd[E]  = 1;
	rgs->xn_pd[E]  = 2;

    }
    if( rgs->inner_id == 3)
    {
	rgs->xp_pd[0] = 0;
	rgs->xp_pd[1] = 1;
	rgs->xp_pd[2] = 2;
	rgs->xp_pd[3] = 3;

	rgs->xn_pd[0] = 0;
	rgs->xn_pd[1] = 1;
	rgs->xn_pd[2] = 2;
	rgs->xn_pd[3] = 4;
	
	rgs->esp_vc[E] = 0;
	rgs->xp_pd[E]  = 1;
	rgs->xn_pd[E]  = 2;
    }

}

#endif

#ifdef NETWORK_TYPE_TORUS 
#define A 0
#define B 1
#define C 2
#define D 3
#define E 4

//typedef enum {A=0, B, C, D, E} drct;


void sw_confg_register_init(sw_config_register * rgs, int index,int *dms)
{
    int i;
//    rgs->inner_id = index % 4;
    i = index;
    rgs->x_id[A] = i % dms[0];
    rgs->x_id[B] = (i / dms[0]) % dms[1];
    rgs->x_id[C] = i / (dms[0]*dms[1]) % dms[2];
    rgs->x_id[D] = i / (dms[0]*dms[1]*dms[2]) % dms[3] ;
//    rgs->x_id[E] = rgs->inner_id;
//    printf("A = %d, B = %d, C = %d, D = %d \n",rgs->x_id[A],rgs->x_id[B],rgs->x_id[C],rgs->x_id[D]);

    for(i = 0; i < 4; i++)
    {
	if(rgs->x_id[i] == 0)
	    rgs->dataline[i] = true;
	else 
	    rgs->dataline[i] = false;
	
	rgs->esp_vc[i] = i;
    }
}
#endif

#ifdef NETWORK_TYPE_FLBFLY

void sw_confg_register_init(sw_config_register * rgs, int index,int *dms)
{
    int k = dms[0];
    int n = dms[1];
    int i;
    for(i = 0; i < n - 1; i++)
    {
	rgs->x_id[i] = index /(int)pow(k,i) % k;
	//printf("index = %d,rgs->x_id[%d]=%d\n",index,i,rgs->x_id[i]);
    }
}

#endif

#ifdef NETWORK_TYPE_SFLBFLY
void sw_confg_register_init(sw_config_register * rgs, int index,int *dms)
{
    int m = dms[0];
    int i;
    int id = 1;
    for(i = 0; i < m ; i++)
    {
	rgs->x_id[i] = index/id % dms[i+1];
	id *= dms[i+1];
    }
}
#endif

#ifdef NETWORK_TYPE_TORFB
void sw_confg_register_init(sw_config_register * rgs, int index,int *dms)
{
    int m = dms[0];
    int i;
    int id = 1;
    for(i = 0; i < m ; i++)
    {
	rgs->x_id[i] = index/id % dms[i+1];
	id *= dms[i+1];
    }
}
#endif

#ifdef NETWORK_TYPE_MESH 
#define A 0
#define B 1
#define C 2
#define D 3
#define E 4

//typedef enum {A=0, B, C, D, E} drct;
void sw_confg_register_init(sw_config_register * rgs, int index,int *dms)
{
    int i;
    i = index;
    rgs->x_id[A] = i % dms[0];
    rgs->x_id[B] = (i / dms[0]) % dms[1];
    rgs->x_id[C] = i / (dms[0]*dms[1]) % dms[2];
    rgs->x_id[D] = i / (dms[0]*dms[1]*dms[2]) % dms[3] ;
    //printf("A = %d, B = %d, C = %d, D = %d \n",rgs->x_id[A],rgs->x_id[B],rgs->x_id[C],rgs->x_id[D]);
}

#endif

#ifdef NETWORK_TYPE_ALLTOALL

//typedef enum {A=0, B, C, D, E} drct;
void sw_confg_register_init(sw_config_register * rgs, int index,int *dms)
{
}

#endif

#ifdef NETWORK_TYPE_TORUS_NEW
#define A 0
#define B 1
#define C 2
#define D 3
#define E 4

//typedef enum {A=0, B, C, D, E} drct;


void sw_confg_register_init(sw_config_register * rgs, int index,int *dms)
{
    int i;
//    rgs->inner_id = index % 4;
    i = index;
    rgs->x_id[A] = i % dms[0];
    rgs->x_id[B] = (i / dms[0]) % dms[1];
    rgs->x_id[C] = i / (dms[0]*dms[1]) % dms[2];
    rgs->x_id[D] = i / (dms[0]*dms[1]*dms[2]) % dms[3] ;
//    rgs->x_id[E] = rgs->inner_id;
    //printf("A = %d, B = %d, C = %d, D = %d \n",rgs->x_id[A],rgs->x_id[B],rgs->x_id[C],rgs->x_id[D]);

    for(i = 0; i < 4; i++)
    {
	if(rgs->x_id[i] == 0)
	    rgs->dataline[i] = true;
	else 
	    rgs->dataline[i] = false;
	
	rgs->esp_vc[i] = i;
    }
}
#endif

#ifdef NETWORK_TYPE_TORUS_DOUBLE_DIMENTION 
#define A 0
#define B 1
#define C 2
#define D 3
#define E 4

//typedef enum {A=0, B, C, D, E} drct;


void sw_confg_register_init(sw_config_register * rgs, int index,int *dms)
{
    int i;
//    rgs->inner_id = index % 4;
    i = index;
    rgs->x_id[A] = i % dms[0];
    rgs->x_id[B] = (i / dms[0]) % dms[1];
    rgs->x_id[C] = i / (dms[0]*dms[1]) % dms[2];
    rgs->x_id[D] = i / (dms[0]*dms[1]*dms[2]) % dms[3] ;
//    rgs->x_id[E] = rgs->inner_id;
    //printf("sw_ index %d  A = %d, B = %d, C = %d, D = %d \n",index, rgs->x_id[A],rgs->x_id[B],rgs->x_id[C],rgs->x_id[D]);

    for(i = 0; i < 4; i++)
    {
	if(rgs->x_id[i] == i)
	    rgs->dataline[i] = true;
	else 
	    rgs->dataline[i] = false;
	
	rgs->esp_vc[i] = i;
    }
}
#endif

#ifdef NETWORK_TYPE_TORUS_DOUBLE_HIGH_LOW 
#define A 0
#define B 1
#define C 2
#define D 3
#define E 4

//typedef enum {A=0, B, C, D, E} drct;


void sw_confg_register_init(sw_config_register * rgs, int index,int *dms)
{
    int i;
//    rgs->inner_id = index % 4;
    i = index;
    rgs->x_id[A] = i % dms[0];
    rgs->x_id[B] = (i / dms[0]) % dms[1];
    rgs->x_id[C] = i / (dms[0]*dms[1]) % dms[2];
    rgs->x_id[D] = i / (dms[0]*dms[1]*dms[2]) % dms[3] ;
//    rgs->x_id[E] = rgs->inner_id;
    //printf("A = %d, B = %d, C = %d, D = %d \n",rgs->x_id[A],rgs->x_id[B],rgs->x_id[C],rgs->x_id[D]);

    for(i = 0; i < 4; i++)
    {
	if(rgs->x_id[i] == 0)
	    rgs->dataline[i] = true;
	else 
	    rgs->dataline[i] = false;
	
	rgs->esp_vc[i] = i;
    }
}
#endif

#ifdef NETWORK_TYPE_D7K_DOUBLE_DEMENSION
#define A 0
#define B 1
#define C 2
#define D 3
#define E 4

//typedef enum {A=0, B, C, D, E} drct;
void sw_confg_register_init(sw_config_register * rgs, int index,int *dms)
{
    int i;
    rgs->inner_id = index % 4;
    i = index / 4;
    rgs->x_id[A] = i % dms[0];
    rgs->x_id[B] = (i / dms[0]) % dms[1];
    rgs->x_id[C] = i / (dms[0]*dms[1]) % dms[2];
    rgs->x_id[D] = i / (dms[0]*dms[1]*dms[2]) % dms[3] ;
    rgs->x_id[E] = rgs->inner_id;
    //printf("A = %d, B = %d, C = %d, D = %d \n",rgs->x_id[A],rgs->x_id[B],rgs->x_id[C],rgs->x_id[D]);

    for(i = 0; i < 4; i++)
    {
	if(rgs->x_id[i] == 0)
	    rgs->dataline[i] = true;
	else 
	    rgs->dataline[i] = false;
	
	rgs->esp_vc[i] = 0;
    }
    if( rgs->inner_id == 0)
    {
	rgs->xp_pd[0] = 3;
	rgs->xp_pd[1] = 0;
	rgs->xp_pd[2] = 1;
	rgs->xp_pd[3] = 2;

	rgs->xn_pd[0] = 4;
	rgs->xn_pd[1] = 0;
	rgs->xn_pd[2] = 1;
	rgs->xn_pd[3] = 2;
	
	rgs->esp_vc[E] = 0;
	rgs->xp_pd[E]  = 1;
	rgs->xn_pd[E]  = 2;
    }
    if( rgs->inner_id == 1)
    {
	rgs->xp_pd[0] = 0;
	rgs->xp_pd[1] = 3;
	rgs->xp_pd[2] = 1;
	rgs->xp_pd[3] = 2;
	
	rgs->xn_pd[0] = 0;
	rgs->xn_pd[1] = 4;
	rgs->xn_pd[2] = 1;
	rgs->xn_pd[3] = 2;

	rgs->esp_vc[E] = 0;
	rgs->xp_pd[E]  = 1;
	rgs->xn_pd[E]  = 2;
    }
    if( rgs->inner_id == 2)
    {
	rgs->xp_pd[0] = 0;
	rgs->xp_pd[1] = 1;
	rgs->xp_pd[2] = 3;
	rgs->xp_pd[3] = 2;

	rgs->xn_pd[0] = 0;
	rgs->xn_pd[1] = 1;
	rgs->xn_pd[2] = 4;
	rgs->xn_pd[3] = 2;

	rgs->esp_vc[E] = 0;
	rgs->xp_pd[E]  = 1;
	rgs->xn_pd[E]  = 2;

    }
    if( rgs->inner_id == 3)
    {
	rgs->xp_pd[0] = 0;
	rgs->xp_pd[1] = 1;
	rgs->xp_pd[2] = 2;
	rgs->xp_pd[3] = 3;

	rgs->xn_pd[0] = 0;
	rgs->xn_pd[1] = 1;
	rgs->xn_pd[2] = 2;
	rgs->xn_pd[3] = 4;
	
	rgs->esp_vc[E] = 0;
	rgs->xp_pd[E]  = 1;
	rgs->xn_pd[E]  = 2;
    }

}

#endif
#ifdef NETWORK_TYPE_D7K_VERSION1
#define A 0
#define B 1
#define C 2
#define D 3
#define E 4

//typedef enum {A=0, B, C, D, E} drct;
void sw_confg_register_init(sw_config_register * rgs, int index,int *dms)
{
    int i;
    rgs->inner_id = index % 4;
    i = index / 4;
    rgs->x_id[A] = i % dms[0];
    rgs->x_id[B] = (i / dms[0]) % dms[1];
    rgs->x_id[C] = i / (dms[0]*dms[1]) % dms[2];
    rgs->x_id[D] = i / (dms[0]*dms[1]*dms[2]) % dms[3] ;
    rgs->x_id[E] = rgs->inner_id;
    //printf("A = %d, B = %d, C = %d, D = %d \n",rgs->x_id[A],rgs->x_id[B],rgs->x_id[C],rgs->x_id[D]);

    for(i = 0; i < 4; i++)
    {
	if(rgs->x_id[i] == 0)
	    rgs->dataline[i] = true;
	else 
	    rgs->dataline[i] = false;
	
	rgs->esp_vc[i] = 0;
    }
    if( rgs->inner_id == 0)
    {
	rgs->xp_pd[0] = 3;
	rgs->xp_pd[1] = 0;
	rgs->xp_pd[2] = 1;
	rgs->xp_pd[3] = 2;

	rgs->xn_pd[0] = 4;
	rgs->xn_pd[1] = 0;
	rgs->xn_pd[2] = 1;
	rgs->xn_pd[3] = 2;
	
	rgs->esp_vc[E] = 0;
	rgs->xp_pd[E]  = 1;
	rgs->xn_pd[E]  = 2;
    }
    if( rgs->inner_id == 1)
    {
	rgs->xp_pd[0] = 0;
	rgs->xp_pd[1] = 3;
	rgs->xp_pd[2] = 1;
	rgs->xp_pd[3] = 2;
	
	rgs->xn_pd[0] = 0;
	rgs->xn_pd[1] = 4;
	rgs->xn_pd[2] = 1;
	rgs->xn_pd[3] = 2;

	rgs->esp_vc[E] = 0;
	rgs->xp_pd[E]  = 1;
	rgs->xn_pd[E]  = 2;
    }
    if( rgs->inner_id == 2)
    {
	rgs->xp_pd[0] = 0;
	rgs->xp_pd[1] = 1;
	rgs->xp_pd[2] = 3;
	rgs->xp_pd[3] = 2;

	rgs->xn_pd[0] = 0;
	rgs->xn_pd[1] = 1;
	rgs->xn_pd[2] = 4;
	rgs->xn_pd[3] = 2;

	rgs->esp_vc[E] = 0;
	rgs->xp_pd[E]  = 1;
	rgs->xn_pd[E]  = 2;

    }
    if( rgs->inner_id == 3)
    {
	rgs->xp_pd[0] = 0;
	rgs->xp_pd[1] = 1;
	rgs->xp_pd[2] = 2;
	rgs->xp_pd[3] = 3;

	rgs->xn_pd[0] = 0;
	rgs->xn_pd[1] = 1;
	rgs->xn_pd[2] = 2;
	rgs->xn_pd[3] = 4;
	
	rgs->esp_vc[E] = 0;
	rgs->xp_pd[E]  = 1;
	rgs->xn_pd[E]  = 2;
    }

}

#endif
#ifdef NETWORK_TYPE_MESH_ADAPTIVE
#define A 0
#define B 1
#define C 2
#define D 3
#define E 4

//typedef enum {A=0, B, C, D, E} drct;
void sw_confg_register_init(sw_config_register * rgs, int index,int *dms)
{
    int i;
    i = index;
    rgs->x_id[A] = i % dms[0];
    rgs->x_id[B] = (i / dms[0]) % dms[1];
    rgs->x_id[C] = i / (dms[0]*dms[1]) % dms[2];
    rgs->x_id[D] = i / (dms[0]*dms[1]*dms[2]) % dms[3] ;
    //printf("A = %d, B = %d, C = %d, D = %d \n",rgs->x_id[A],rgs->x_id[B],rgs->x_id[C],rgs->x_id[D]);
}

#endif
