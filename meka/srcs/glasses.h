//-----------------------------------------------------------------------------
// MEKA - glasses.h
// 3D Glasses Support and Emulation - Headers
//-----------------------------------------------------------------------------

#ifndef __GLASSES_H__
#define __GLASSES_H__

//-----------------------------------------------------------------------------
// Definitions
//-----------------------------------------------------------------------------

// Options for t_glasses.Status
#define GLASSES_STATUS_ENABLED          (1)
#define GLASSES_STATUS_DISABLED         (0)
#define GLASSES_STATUS_AUTO             (2)

// Options for t_glasses.Mode
#define GLASSES_MODE_SHOW_BOTH          (0)
#define GLASSES_MODE_SHOW_ONLY_LEFT     (1)
#define GLASSES_MODE_SHOW_ONLY_RIGHT    (2)
#define GLASSES_MODE_COM_PORT           (3)
#define GLASSES_MODE_SBS                (4)


//-----------------------------------------------------------------------------
// Data
//-----------------------------------------------------------------------------

struct t_glasses
{
    int         Enabled;
    bool		GameSupports3d;
    int         Mode;
    int         ComPort;
    #ifdef ARCH_WIN32
    HANDLE      ComHandle;
    #endif
};

extern t_glasses   Glasses;

//-----------------------------------------------------------------------------
// Functions
//-----------------------------------------------------------------------------

void    Glasses_Init_Values             (void);
void    Glasses_Close                   (void);
int     Glasses_Must_Skip_Frame         (void);
void    Glasses_Write                   (int LeftEnable, int RightEnable);
void    Glasses_Update                  (void);

void    Glasses_Set_Status_initial      (int status);
void    Glasses_Force_Update            (void);
void    Glasses_Set_Status              (t_menu_event *event);
void    Glasses_Set_Mode                (int mode);
void    Glasses_Set_ComPort             (int port);
void    Glasses_Switch_Mode_Show_Both   (void);
void    Glasses_Switch_Mode_Show_Left   (void);
void    Glasses_Switch_Mode_Show_Right  (void);
void    Glasses_Switch_Mode_Com_Port    (void);
void    Glasses_Switch_Mode_SBS         (void);

//-----------------------------------------------------------------------------


#endif
