#ifndef SW_CONFIG_REGISTER
#define SW_CONFIG_REGISTER
#include<stdio.h>
#include<stdbool.h>
#include"macro_select.h"

#define MAX_FLY 10

typedef struct 
{
    int  inner_id;
    bool dataline[5];
    int  x_id[5];
    int esp_vc[5];
    unsigned int xp_pd[5];
    unsigned int xn_pd[5];
}sw_config_register_d7k;

typedef struct
{
    //int temp;
}sw_config_register_ftree;

typedef struct
{
    int  x_id[5];
}sw_config_register_mesh;

typedef struct 
{
    int  inner_id;
    bool dataline[5];
    int  x_id[5];
    int esp_vc[5];
}sw_config_register_torus;

typedef struct
{
    int x_id[MAX_FLY];
}sw_config_register_flbfly;

typedef struct
{
}sw_config_register_alltoall;

#ifdef NETWORK_TYPE_FTREE
typedef sw_config_register_ftree sw_config_register;
#endif
#ifdef NETWORK_TYPE_TORFT
typedef sw_config_register_ftree sw_config_register;
#endif
#ifdef NETWORK_TYPE_VTORFT
typedef sw_config_register_ftree sw_config_register;
#endif
#ifdef NETWORK_TYPE_FLBFLY
typedef sw_config_register_flbfly sw_config_register;
#endif
#ifdef NETWORK_TYPE_SFLBFLY
typedef sw_config_register_flbfly sw_config_register;
#endif
#ifdef NETWORK_TYPE_TORFB
typedef sw_config_register_flbfly sw_config_register;
#endif
#ifdef NETWORK_TYPE_D7K
typedef sw_config_register_d7k sw_config_register;
#endif
#ifdef NETWORK_TYPE_D7K_DOUBLE_DEMENSION
typedef sw_config_register_d7k sw_config_register;
#endif
#ifdef NETWORK_TYPE_D7K_VERSION1
typedef sw_config_register_d7k sw_config_register;
#endif
#ifdef NETWORK_TYPE_TORUS
typedef sw_config_register_torus sw_config_register;
#endif
#ifdef NETWORK_TYPE_MESH
typedef sw_config_register_mesh sw_config_register;
#endif
#ifdef NETWORK_TYPE_ALLTOALL
typedef sw_config_register_alltoall sw_config_register;
#endif
#ifdef NETWORK_TYPE_TORUS_NEW
typedef sw_config_register_torus sw_config_register;
#endif
#ifdef NETWORK_TYPE_TORUS_DOUBLE_DIMENTION 
typedef sw_config_register_torus sw_config_register;
#endif
#ifdef NETWORK_TYPE_TORUS_DOUBLE_HIGH_LOW 
typedef sw_config_register_torus sw_config_register;
#endif
#ifdef NETWORK_TYPE_MESH_ADAPTIVE
typedef sw_config_register_mesh sw_config_register;
#endif

extern sw_config_register * sw_crgs;

extern void sw_confg_regiser_create(int swnum);
extern void sw_confg_register_init(sw_config_register * rgs, int index,int *dms);


#endif
