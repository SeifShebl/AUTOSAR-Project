 /******************************************************************************
 *
 * Module: Port
 *
 * File Name: Port.c
 *
 * Description: Source file for TM4C123GH6PM Microcontroller - Port Driver.
 *
 * Author: Mohamed Tarek
 ******************************************************************************/
#include "Port.h"
#include "Port_Regs.h"
#include "tm4c123gh6pm_registers.h"

     #if (PORT_DEV_ERROR_DETECT == STD_ON)

#include "Det.h"
/* AUTOSAR Version checking between Det and Port Modules */
#if ((DET_AR_MAJOR_VERSION != PORT_AR_RELEASE_MAJOR_VERSION)\
 || (DET_AR_MINOR_VERSION != PORT_AR_RELEASE_MINOR_VERSION)\
 || (DET_AR_PATCH_VERSION != PORT_AR_RELEASE_PATCH_VERSION))
  #error "The AR version of Det.h does not match the expected version"
#endif

#endif

STATIC const Port_ConfigPin *Port_ConfigPtr = NULL_PTR;

STATIC uint8 Port_Status = PORT_NOT_INITIALIZED;

/************************************************************************************
* Service Name: Port_Init
* Service ID[hex]: 0x00
* Sync/Async: Synchronous
* Reentrancy: Non reentrant
* Parameters (in): ConfigPtr - Pointer to configuration set
* Parameters (inout): None
* Parameters (out): None
* Return value: None
* Description: Function to Initialize the Port Driver module.
************************************************************************************/

void Port_Init( const Port_ConfigType* ConfigPtr ) {
  #if (PORT_DEV_ERROR_DETECT == STD_ON)
	/* check if the input configuration pointer is not a NULL_PTR */
	if (NULL_PTR == ConfigPtr)
	{
		Det_ReportError(PORT_MODULE_ID, PORT_INSTANCE_ID, PORT_INIT_SID,
		     PORT_E_PARAM_CONFIG);
	}
	else
#endif
	{
		/*
		 * Set the module state to initialized and point to the PB configuration structure using a global pointer.
		 * This global pointer is global to be used by other functions to read the PB configuration structures
		 */
		Port_Status       = PORT_INITIALIZED;
		Port_ConfigPtr = ConfigPtr->Pin; /* address of the first Channels structure --> Channels[0] */
	}
        volatile uint32 *PORT_BaseAddressPtr = NULL_PTR;
     
        volatile uint32 delay = 0;
        
        for(Port_PinType index = 0; index < PORT_CONFIGURED_PORTPINS; index++){
    
    switch(Port_ConfigPtr[index].port_num)
    {
        case  0: PORT_BaseAddressPtr = (volatile uint32 *)GPIO_PORTA_BASE_ADDRESS; /* PORTA Base Address */
		 break;
	case  1: PORT_BaseAddressPtr = (volatile uint32 *)GPIO_PORTB_BASE_ADDRESS; /* PORTB Base Address */
		 break;
	case  2: PORT_BaseAddressPtr = (volatile uint32 *)GPIO_PORTC_BASE_ADDRESS; /* PORTC Base Address */
		 break;
	case  3: PORT_BaseAddressPtr = (volatile uint32 *)GPIO_PORTD_BASE_ADDRESS; /* PORTD Base Address */
		 break;
        case  4: PORT_BaseAddressPtr = (volatile uint32 *)GPIO_PORTE_BASE_ADDRESS; /* PORTE Base Address */
		 break;
        case  5: PORT_BaseAddressPtr = (volatile uint32 *)GPIO_PORTF_BASE_ADDRESS; /* PORTF Base Address */
		 break;
    }
    
    /* Enable clock for PORT and allow time for clock to start*/
    SYSCTL_REGCGC2_REG |= (1<<Port_ConfigPtr[index].port_num);
    delay = SYSCTL_REGCGC2_REG;
    
    if( ((Port_ConfigPtr[index].port_num == PORT_PORTD_ID) && (Port_ConfigPtr[index].pin_num == PORT_PIN7_ID)) || ((Port_ConfigPtr[index].port_num == PORT_PORTF_ID) && (Port_ConfigPtr[index].pin_num == PORT_PIN0_ID)) ) /* PD7 or PF0 */
    {
        *(volatile uint32 *)((volatile uint8 *)PORT_BaseAddressPtr + PORT_LOCK_REG_OFFSET) = 0x4C4F434B;                     /* Unlock the GPIOCR register */   
        SET_BIT(*(volatile uint32 *)((volatile uint8 *)PORT_BaseAddressPtr + PORT_COMMIT_REG_OFFSET) , Port_ConfigPtr[index].pin_num);  /* Set the corresponding bit in GPIOCR register to allow changes on this pin */
    }
    else if( (Port_ConfigPtr[index].port_num == PORT_PORTC_ID) && (Port_ConfigPtr[index].pin_num <= PORT_PIN3_ID) ) /* PC0 to PC3 */
    {
        /* Do Nothing ...  this is the JTAG pins */
      continue;
    }
    else
    {
        /* Do Nothing ... No need to unlock the commit register for this pin */
    }
    
    if (Port_ConfigPtr[index].pin_mode == PORT_PIN_MODE_DIO)
    {     
    CLEAR_BIT(*(volatile uint32 *)((volatile uint8 *)PORT_BaseAddressPtr + PORT_ANALOG_MODE_SEL_REG_OFFSET) , Port_ConfigPtr[index].pin_num);      /* Clear the corresponding bit in the GPIOAMSEL register to disable analog functionality on this pin */
   
    CLEAR_BIT(*(volatile uint32 *)((volatile uint8 *)PORT_BaseAddressPtr + PORT_ALT_FUNC_REG_OFFSET) , Port_ConfigPtr[index].pin_num);             /* Disable Alternative function for this pin by clear the corresponding bit in GPIOAFSEL register */
   
    *(volatile uint32 *)((volatile uint8 *)PORT_BaseAddressPtr + PORT_CTL_REG_OFFSET) &= ~(0x0000000F << (Port_ConfigPtr[index].pin_num * 4));     /* Clear the PMCx bits for this pin */
    
    SET_BIT(*(volatile uint32 *)((volatile uint8 *)PORT_BaseAddressPtr + PORT_DIGITAL_ENABLE_REG_OFFSET) , Port_ConfigPtr[index].pin_num);        /* Set the corresponding bit in the GPIODEN register to enable digital functionality on this pin */
    }
    
    else if (Port_ConfigPtr[index].pin_mode == PORT_PIN_MODE_ADC)
    {
      SET_BIT(*(volatile uint32 *)((volatile uint8 *)PORT_BaseAddressPtr + PORT_ANALOG_MODE_SEL_REG_OFFSET) , Port_ConfigPtr[index].pin_num);      /* Set the corresponding bit in the GPIOAMSEL register to enable analog functionality on this pin */
   
      SET_BIT(*(volatile uint32 *)((volatile uint8 *)PORT_BaseAddressPtr + PORT_ALT_FUNC_REG_OFFSET) , Port_ConfigPtr[index].pin_num);           /* Enable Alternative function for this pin by clear the corresponding bit in GPIOAFSEL register */
   
      *(volatile uint32 *)((volatile uint8 *)PORT_BaseAddressPtr + PORT_CTL_REG_OFFSET) |= (0x0000000F & Port_ConfigPtr[index].pin_mode << (Port_ConfigPtr[index].pin_num * 4));     /* Set the PMCx bits for this pin */
 
      CLEAR_BIT(*(volatile uint32 *)((volatile uint8 *)PORT_BaseAddressPtr + PORT_DIGITAL_ENABLE_REG_OFFSET) , Port_ConfigPtr[index].pin_num);     /* Clear the corresponding bit in the GPIODEN register to enable digital functionality on this pin */
        
    }
    
    else
    {
      CLEAR_BIT(*(volatile uint32 *)((volatile uint8 *)PORT_BaseAddressPtr + PORT_ANALOG_MODE_SEL_REG_OFFSET) , Port_ConfigPtr[index].pin_num);    /* Clear the corresponding bit in the GPIOAMSEL register to disable analog functionality on this pin */
    
      SET_BIT(*(volatile uint32 *)((volatile uint8 *)PORT_BaseAddressPtr + PORT_ALT_FUNC_REG_OFFSET) , Port_ConfigPtr[index].pin_num);             /* Enable Alternative function for this pin by clear the corresponding bit in GPIOAFSEL register */
  
      *(volatile uint32 *)((volatile uint8 *)PORT_BaseAddressPtr + PORT_CTL_REG_OFFSET) |= (0x0000000F & Port_ConfigPtr[index].pin_mode << (Port_ConfigPtr[index].pin_num * 4));     /* Set the PMCx bits for this pin */

      SET_BIT(*(volatile uint32 *)((volatile uint8 *)PORT_BaseAddressPtr + PORT_DIGITAL_ENABLE_REG_OFFSET) , Port_ConfigPtr[index].pin_num);       /* Set the corresponding bit in the GPIODEN register to enable digital functionality on this pin */
    }
    
    /******************************************DIRECTION CHECK****************************************************************/
    
    if(Port_ConfigPtr[index].direction == PORT_PIN_OUT)
    {
	SET_BIT(*(volatile uint32 *)((volatile uint8 *)PORT_BaseAddressPtr + PORT_DIR_REG_OFFSET) ,Port_ConfigPtr[index].pin_num);                /* Set the corresponding bit in the GPIODIR register to configure it as output pin */
        
        if(Port_ConfigPtr[index].initial_value == STD_HIGH)
        {
            SET_BIT(*(volatile uint32 *)((volatile uint8 *)PORT_BaseAddressPtr + PORT_DATA_REG_OFFSET) , Port_ConfigPtr[index].pin_num);          /* Set the corresponding bit in the GPIODATA register to provide initial value 1 */
        }
        else
        {
            CLEAR_BIT(*(volatile uint32 *)((volatile uint8 *)PORT_BaseAddressPtr + PORT_DATA_REG_OFFSET) , Port_ConfigPtr[index].pin_num);        /* Clear the corresponding bit in the GPIODATA register to provide initial value 0 */
        }
    }
    else if(Port_ConfigPtr[index].direction == PORT_PIN_IN)
    {
        CLEAR_BIT(*(volatile uint32 *)((volatile uint8 *)PORT_BaseAddressPtr + PORT_DIR_REG_OFFSET) ,Port_ConfigPtr[index].pin_num);             /* Clear the corresponding bit in the GPIODIR register to configure it as input pin */
        
        if(Port_ConfigPtr[index].resistor == PORT_RESISTOR_PULL_UP)
        {
            SET_BIT(*(volatile uint32 *)((volatile uint8 *)PORT_BaseAddressPtr + PORT_PULL_UP_REG_OFFSET) , Port_ConfigPtr[index].pin_num);       /* Set the corresponding bit in the GPIOPUR register to enable the internal pull up pin */
        }
        else if(Port_ConfigPtr[index].resistor == PORT_RESISTOR_PULL_DOWN)
        {
            SET_BIT(*(volatile uint32 *)((volatile uint8 *)PORT_BaseAddressPtr + PORT_PULL_DOWN_REG_OFFSET) ,Port_ConfigPtr[index].pin_num);     /* Set the corresponding bit in the GPIOPDR register to enable the internal pull down pin */
        }
        else
        {
            CLEAR_BIT(*(volatile uint32 *)((volatile uint8 *)PORT_BaseAddressPtr + PORT_PULL_UP_REG_OFFSET) ,Port_ConfigPtr[index].pin_num);     /* Clear the corresponding bit in the GPIOPUR register to disable the internal pull up pin */
           
            CLEAR_BIT(*(volatile uint32 *)((volatile uint8 *)PORT_BaseAddressPtr + PORT_PULL_DOWN_REG_OFFSET) ,Port_ConfigPtr[index].pin_num);   /* Clear the corresponding bit in the GPIOPDR register to disable the internal pull down pin */
        }
    }
    else
    {
        /* Do Nothing */
    }
       
}
}


/************************************************************************************
* Service Name: Port_SetPinDirection
* Service ID[hex]: 0x01
* Sync/Async: Synchronous
* Reentrancy: reentrant
* Parameters (in): Pin - Port Pin ID number , Direction - Port Pin Direction
* Parameters (inout): None
* Parameters (out): None
* Return value: None
* Description: Function to Sets the port pin direction.
************************************************************************************/
#if (PORT_SET_PIN_DIRECTION_API == STD_ON)                  
void Port_SetPinDirection( Port_PinType Pin, Port_PinDirectionType Direction )
{
#if (PORT_DEV_ERROR_DETECT == STD_ON)
  /* check if the input configuration pointer is not a NULL_PTR */
  if (Port_Status == PORT_NOT_INITIALIZED)
  {
    Det_ReportError(PORT_MODULE_ID, PORT_INSTANCE_ID, PORT_SET_PIN_DIRECTION_SID,
                    PORT_E_UNINIT);
  }
  else if (Pin >= PORT_CONFIGURED_PORTPINS)
  {
    Det_ReportError(PORT_MODULE_ID, PORT_INSTANCE_ID, PORT_SET_PIN_DIRECTION_SID,
                    PORT_E_PARAM_PIN);
  }
   else if  (Port_ConfigPtr[Pin].PORT_PIN_DIRECTION_CHANGEABLE == STD_OFF)
   {
     Det_ReportError(PORT_MODULE_ID, PORT_INSTANCE_ID, PORT_SET_PIN_DIRECTION_SID,
                     PORT_E_DIRECTION_UNCHANGEABLE);
   }
  
#endif
  volatile uint32 *PORT_BaseAddressPtr = NULL_PTR;
  
   switch(Port_ConfigPtr[Pin].port_num)
    {
        case  0: PORT_BaseAddressPtr = (volatile uint32 *)GPIO_PORTA_BASE_ADDRESS; /* PORTA Base Address */
		 break;
	case  1: PORT_BaseAddressPtr = (volatile uint32 *)GPIO_PORTB_BASE_ADDRESS; /* PORTB Base Address */
		 break;
	case  2: PORT_BaseAddressPtr = (volatile uint32 *)GPIO_PORTC_BASE_ADDRESS; /* PORTC Base Address */
		 break;
	case  3: PORT_BaseAddressPtr = (volatile uint32 *)GPIO_PORTD_BASE_ADDRESS; /* PORTD Base Address */
		 break;
        case  4: PORT_BaseAddressPtr = (volatile uint32 *)GPIO_PORTE_BASE_ADDRESS; /* PORTE Base Address */
		 break;
        case  5: PORT_BaseAddressPtr = (volatile uint32 *)GPIO_PORTF_BASE_ADDRESS; /* PORTF Base Address */
		 break;
    }
      if( ((Port_ConfigPtr[Pin].port_num == PORT_PORTD_ID) && (Port_ConfigPtr[Pin].pin_num == PORT_PIN7_ID)) || ((Port_ConfigPtr[Pin].port_num == PORT_PORTF_ID) && (Port_ConfigPtr[Pin].pin_num == PORT_PIN0_ID)) ) /* PD7 or PF0 */
    {
        *(volatile uint32 *)((volatile uint8 *)PORT_BaseAddressPtr + PORT_LOCK_REG_OFFSET) = 0x4C4F434B;                     /* Unlock the GPIOCR register */   
        SET_BIT(*(volatile uint32 *)((volatile uint8 *)PORT_BaseAddressPtr + PORT_COMMIT_REG_OFFSET) , Port_ConfigPtr[Pin].pin_num);  /* Set the corresponding bit in GPIOCR register to allow changes on this pin */
    }
    else if( (Port_ConfigPtr[Pin].port_num == PORT_PORTC_ID) && (Port_ConfigPtr[Pin].pin_num <= PORT_PIN3_ID) ) /* PC0 to PC3 */
    {
        /* Do Nothing ...  this is the JTAG pins */
      return;
    }
    else
    {
        /* Do Nothing ... No need to unlock the commit register for this pin */
    }
 /****************************DIRECTION CHANGE************************************************************/
    if(Direction == PORT_PIN_OUT)
    {
	SET_BIT(*(volatile uint32 *)((volatile uint8 *)PORT_BaseAddressPtr + PORT_DIR_REG_OFFSET) ,Port_ConfigPtr[Pin].pin_num);                /* Set the corresponding bit in the GPIODIR register to configure it as output pin */
        
        if(Port_ConfigPtr[Pin].initial_value == STD_HIGH)
        {
            SET_BIT(*(volatile uint32 *)((volatile uint8 *)PORT_BaseAddressPtr + PORT_DATA_REG_OFFSET) , Port_ConfigPtr[Pin].pin_num);          /* Set the corresponding bit in the GPIODATA register to provide initial value 1 */
        }
        else
        {
            CLEAR_BIT(*(volatile uint32 *)((volatile uint8 *)PORT_BaseAddressPtr + PORT_DATA_REG_OFFSET) , Port_ConfigPtr[Pin].pin_num);        /* Clear the corresponding bit in the GPIODATA register to provide initial value 0 */
        }
    }
    else if(Direction == PORT_PIN_IN)
    {
        CLEAR_BIT(*(volatile uint32 *)((volatile uint8 *)PORT_BaseAddressPtr + PORT_DIR_REG_OFFSET) ,Port_ConfigPtr[Pin].pin_num);             /* Clear the corresponding bit in the GPIODIR register to configure it as input pin */
        
        if(Port_ConfigPtr[Pin].resistor == PORT_RESISTOR_PULL_UP)
        {
            SET_BIT(*(volatile uint32 *)((volatile uint8 *)PORT_BaseAddressPtr + PORT_PULL_UP_REG_OFFSET) , Port_ConfigPtr[Pin].pin_num);       /* Set the corresponding bit in the GPIOPUR register to enable the internal pull up pin */
        }
        else if(Port_ConfigPtr[Pin].resistor == PORT_RESISTOR_PULL_DOWN)
        {
            SET_BIT(*(volatile uint32 *)((volatile uint8 *)PORT_BaseAddressPtr + PORT_PULL_DOWN_REG_OFFSET) ,Port_ConfigPtr[Pin].pin_num);     /* Set the corresponding bit in the GPIOPDR register to enable the internal pull down pin */
        }
        else
        {
            CLEAR_BIT(*(volatile uint32 *)((volatile uint8 *)PORT_BaseAddressPtr + PORT_PULL_UP_REG_OFFSET) ,Port_ConfigPtr[Pin].pin_num);     /* Clear the corresponding bit in the GPIOPUR register to disable the internal pull up pin */
           
            CLEAR_BIT(*(volatile uint32 *)((volatile uint8 *)PORT_BaseAddressPtr + PORT_PULL_DOWN_REG_OFFSET) ,Port_ConfigPtr[Pin].pin_num);   /* Clear the corresponding bit in the GPIOPDR register to disable the internal pull down pin */
        }
    }
    else
    {
        /* Do Nothing */
    }
 #endif
} 

/************************************************************************************
* Service Name: Port_RefreshPortDirection
* Service ID[hex]: 0x02
* Sync/Async: Synchronous
* Reentrancy: Non reentrant
* Parameters (in): None
* Parameters (inout): None
* Parameters (out): None
* Return value: None
* Description: Refreshes port direction.
************************************************************************************/
void Port_RefreshPortDirection( void )
{
#if (PORT_DEV_ERROR_DETECT == STD_ON)
  /* check if the input configuration pointer is not a NULL_PTR */
  if (Port_Status == PORT_NOT_INITIALIZED)
  {
    Det_ReportError(PORT_MODULE_ID, PORT_INSTANCE_ID, PORT_REFRESH_PORT_DIR_SID,
                    PORT_E_UNINIT);
  }
#endif
  
  for(Port_PinType index = 0; index < PORT_CONFIGURED_PORTPINS; index++)
  {
    
    volatile uint32 *PORT_BaseAddressPtr = NULL_PTR;
    
    switch(Port_ConfigPtr[index].port_num )
    {
    case  0: PORT_BaseAddressPtr = (volatile uint32 *)GPIO_PORTA_BASE_ADDRESS; /* PORTA Base Address */
    break;
    case  1: PORT_BaseAddressPtr = (volatile uint32 *)GPIO_PORTB_BASE_ADDRESS; /* PORTB Base Address */
    break;
    case  2: PORT_BaseAddressPtr = (volatile uint32 *)GPIO_PORTC_BASE_ADDRESS; /* PORTC Base Address */
    break;
    case  3: PORT_BaseAddressPtr = (volatile uint32 *)GPIO_PORTD_BASE_ADDRESS; /* PORTD Base Address */
    break;
    case  4: PORT_BaseAddressPtr = (volatile uint32 *)GPIO_PORTE_BASE_ADDRESS; /* PORTE Base Address */
    break;
    case  5: PORT_BaseAddressPtr = (volatile uint32 *)GPIO_PORTF_BASE_ADDRESS; /* PORTF Base Address */
    break;
    }
      if ( ((Port_ConfigPtr[index].port_num == PORT_PORTD_ID) && (Port_ConfigPtr[index].pin_num == PORT_PIN7_ID )) || ((Port_ConfigPtr[index].port_num == PORT_PORTF_ID) && (Port_ConfigPtr[index].pin_num == PORT_PIN0_ID )) )/* PD7 or PF0 */
    {
      /* Unlock the GPIOCR register */
      *(volatile uint32 *)((volatile uint8 *)PORT_BaseAddressPtr + PORT_LOCK_REG_OFFSET) = 0x4C4F434B;/* Unlock the GPIOCR register */
      SET_BIT(*(volatile uint32 *)((volatile uint8 *)PORT_BaseAddressPtr + PORT_COMMIT_REG_OFFSET) , Port_ConfigPtr[index].pin_num);  /* Set the corresponding bit in GPIOCR register to allow changes on this pin */
    }
    else if ( ((Port_ConfigPtr[index].port_num == PORT_PORTC_ID) && (Port_ConfigPtr[index].pin_num <= PORT_PIN3_ID ))) /* PC0 to PC3 */
    {
      /* Do Nothing ...  this is the JTAG pins */
    }
    else
    {
      /* Do Nothing ... No need to unlock the commit register for this pin */
    }
    if (Port_ConfigPtr[index].PORT_PIN_DIRECTION_CHANGEABLE == STD_OFF)
    {
      if(Port_ConfigPtr[index].direction== PORT_PIN_OUT)
      {
        /* Set the corresponding bit in the GPIODIR register to configure it as output pin */
        SET_BIT(*(volatile uint32 *)((volatile uint8 *)PORT_BaseAddressPtr + PORT_DIR_REG_OFFSET) , Port_ConfigPtr[index].pin_num);
      }
      else if(Port_ConfigPtr[index].direction == PORT_PIN_IN)
      {
        /* Clear the corresponding bit in the GPIODIR register to configure it as input pin */
        CLEAR_BIT(*(volatile uint32 *)((volatile uint8 *)PORT_BaseAddressPtr + PORT_DIR_REG_OFFSET) , Port_ConfigPtr[index].pin_num);
      }
      else
      {	
        /* Do Nothing */	
      }
    }
    
  }
  }


/************************************************************************************
* Service Name: Port_GetVersionInfo
* Service ID[hex]: 0x03
* Sync/Async: Synchronous
* Reentrancy: Reentrant
* Parameters (in): None
* Parameters (inout): None
* Parameters (out): VersionInfo - Pointer to where to store the version information of this module.
* Return value: None
* Description: Function to get the version information of this module.
************************************************************************************/
#if (PORT_VERSION_INFO_API == STD_ON)
  void Port_GetVersionInfo(Std_VersionInfoType* versioninfo)
  {
#if (PORT_DEV_ERROR_DETECT == STD_ON)
    /* check if the input configuration pointer is not a NULL_PTR */
    if(versioninfo == NULL_PTR)
    {
      Det_ReportError(PORT_MODULE_ID, PORT_INSTANCE_ID, PORT_GET_VERSION_INFO_SID, PORT_E_PARAM_POINTER);
    }
    else
    {	/* Do Nothing */	}
    
    /* Check if the Driver is initialized before using this function */
    if(Port_Status == PORT_NOT_INITIALIZED)
    {
      Det_ReportError(PORT_MODULE_ID, PORT_INSTANCE_ID, PORT_GET_VERSION_INFO_SID, PORT_E_UNINIT);
    }
    else
    {	/* Do Nothing */	}
#endif
    
    /* Copy the module Id */
    versioninfo->moduleID = (uint16)PORT_MODULE_ID;
    /* Copy the vendor Id */
    versioninfo->vendorID = (uint16)PORT_VENDOR_ID;
    /* Copy Software Major Version */
    versioninfo->sw_major_version = (uint8)PORT_SW_MAJOR_VERSION;
    /* Copy Software Minor Version */
    versioninfo->sw_minor_version = (uint8)PORT_SW_MINOR_VERSION;
    /* Copy Software Patch Version */
    versioninfo->sw_patch_version = (uint8)PORT_SW_PATCH_VERSION;
  }
#endif


/************************************************************************************
* Service Name: Port_SetPinMode
* Service ID[hex]: 0x04
* Sync/Async: Synchronous
* Reentrancy: reentrant
* Parameters (in): Pin - Port Pin ID number, Mode - New Port Pin mode to be set on port pin
* Parameters (inout): None
* Parameters (out): None
* Return value: None
* Description: Sets the port pin mode.
************************************************************************************/
#if (PORT_SET_PIN_MODE_API == STD_ON)
void Port_SetPinMode(Port_PinType Pin, Port_PinModeType Mode)
{
#if (PORT_DEV_ERROR_DETECT == STD_ON)
  /* Check if the Driver is initialized before using this function */
  if(Port_Status == PORT_NOT_INITIALIZED)
  {
    Det_ReportError(PORT_MODULE_ID, PORT_INSTANCE_ID, PORT_SET_PIN_MODE_SID, PORT_E_UNINIT);
  }
  else
  {	/* Do Nothing */	}
  
  /* check if incorrect Port Pin ID passed */
  if(Pin >= PORT_CONFIGURED_PORTPINS)
  {
    Det_ReportError(PORT_MODULE_ID, PORT_INSTANCE_ID, PORT_SET_PIN_MODE_SID, PORT_E_PARAM_PIN);
  }
  else
  {	/* Do Nothing */	}
  
  /* check if the Port Pin Mode passed not valid */
  if(Mode > PORT_PIN_MODE_DIO)
  {
    Det_ReportError(PORT_MODULE_ID, PORT_INSTANCE_ID, PORT_SET_PIN_MODE_SID, PORT_E_PARAM_INVALID_MODE);
  }
  else
  {	/* Do Nothing */	}
  
  /* check if the API called when the mode is unchangeable */
  if(Port_ConfigPtr[Pin].PORT_PIN_MODE_CHANGEABLE == STD_OFF)
  {
    Det_ReportError(PORT_MODULE_ID, PORT_INSTANCE_ID, PORT_SET_PIN_MODE_SID, PORT_E_MODE_UNCHANGEABLE);
  }
  else
  {	/* Do Nothing */	}
#endif
volatile uint32 *PORT_BaseAddressPtr = NULL_PTR;
 switch(Port_ConfigPtr[Pin].port_num )
  {
  case  0: PORT_BaseAddressPtr = (volatile uint32 *)GPIO_PORTA_BASE_ADDRESS; /* PORTA Base Address */
  break;
  case  1: PORT_BaseAddressPtr = (volatile uint32 *)GPIO_PORTB_BASE_ADDRESS; /* PORTB Base Address */
  break;
  case  2: PORT_BaseAddressPtr = (volatile uint32 *)GPIO_PORTC_BASE_ADDRESS; /* PORTC Base Address */
  break;
  case  3: PORT_BaseAddressPtr = (volatile uint32 *)GPIO_PORTD_BASE_ADDRESS; /* PORTD Base Address */
  break;
  case  4: PORT_BaseAddressPtr = (volatile uint32 *)GPIO_PORTE_BASE_ADDRESS; /* PORTE Base Address */
  break;
  case  5: PORT_BaseAddressPtr = (volatile uint32 *)GPIO_PORTF_BASE_ADDRESS; /* PORTF Base Address */
  break;
  }
  
  if ( ((Port_ConfigPtr[Pin].port_num == PORT_PORTD_ID) && (Port_ConfigPtr[Pin].pin_num == PORT_PIN7_ID )) || ((Port_ConfigPtr[Pin].port_num == PORT_PORTF_ID) && (Port_ConfigPtr[Pin].pin_num == PORT_PIN0_ID )) )/* PD7 or PF0 */
  {
    /* Unlock the GPIOCR register */
    *(volatile uint32 *)((volatile uint8 *)PORT_BaseAddressPtr + PORT_LOCK_REG_OFFSET) = 0x4C4F434B;/* Unlock the GPIOCR register */
    SET_BIT(*(volatile uint32 *)((volatile uint8 *)PORT_BaseAddressPtr + PORT_COMMIT_REG_OFFSET) , Port_ConfigPtr[Pin].pin_num);  /* Set the corresponding bit in GPIOCR register to allow changes on this pin */
  }
  else if ( ((Port_ConfigPtr[Pin].port_num == PORT_PORTC_ID) && (Port_ConfigPtr[Pin].pin_num <= PORT_PIN3_ID ))) /* PC0 to PC3 */
  {
    /* Do Nothing ...  this is the JTAG pins */
    return;
  }
  else
  {
    /* Do Nothing ... No need to unlock the commit register for this pin */
  }
 
 /*****************************CHECK MODE*****************************************************/
    if (Port_ConfigPtr[Pin].pin_mode == PORT_PIN_MODE_DIO)
    {     
    CLEAR_BIT(*(volatile uint32 *)((volatile uint8 *)PORT_BaseAddressPtr + PORT_ANALOG_MODE_SEL_REG_OFFSET) , Port_ConfigPtr[Pin].pin_num);      /* Clear the corresponding bit in the GPIOAMSEL register to disable analog functionality on this pin */
   
    CLEAR_BIT(*(volatile uint32 *)((volatile uint8 *)PORT_BaseAddressPtr + PORT_ALT_FUNC_REG_OFFSET) , Port_ConfigPtr[Pin].pin_num);             /* Disable Alternative function for this pin by clear the corresponding bit in GPIOAFSEL register */
   
    *(volatile uint32 *)((volatile uint8 *)PORT_BaseAddressPtr + PORT_CTL_REG_OFFSET) &= ~(0x0000000F << (Port_ConfigPtr[Pin].pin_num * 4));     /* Clear the PMCx bits for this pin */
    
    SET_BIT(*(volatile uint32 *)((volatile uint8 *)PORT_BaseAddressPtr + PORT_DIGITAL_ENABLE_REG_OFFSET) , Port_ConfigPtr[Pin].pin_num);        /* Set the corresponding bit in the GPIODEN register to enable digital functionality on this pin */
    }
    
    else if (Port_ConfigPtr[Pin].pin_mode == PORT_PIN_MODE_ADC)
    {
      SET_BIT(*(volatile uint32 *)((volatile uint8 *)PORT_BaseAddressPtr + PORT_ANALOG_MODE_SEL_REG_OFFSET) , Port_ConfigPtr[Pin].pin_num);      /* Set the corresponding bit in the GPIOAMSEL register to enable analog functionality on this pin */
   
      SET_BIT(*(volatile uint32 *)((volatile uint8 *)PORT_BaseAddressPtr + PORT_ALT_FUNC_REG_OFFSET) , Port_ConfigPtr[Pin].pin_num);           /* Enable Alternative function for this pin by clear the corresponding bit in GPIOAFSEL register */
   
      *(volatile uint32 *)((volatile uint8 *)PORT_BaseAddressPtr + PORT_CTL_REG_OFFSET) |= (0x0000000F & Port_ConfigPtr[Pin].pin_mode << (Port_ConfigPtr[Pin].pin_num * 4));     /* Set the PMCx bits for this pin */
 
      CLEAR_BIT(*(volatile uint32 *)((volatile uint8 *)PORT_BaseAddressPtr + PORT_DIGITAL_ENABLE_REG_OFFSET) , Port_ConfigPtr[Pin].pin_num);     /* Clear the corresponding bit in the GPIODEN register to enable digital functionality on this pin */
        
    }
    
    else
    {
      CLEAR_BIT(*(volatile uint32 *)((volatile uint8 *)PORT_BaseAddressPtr + PORT_ANALOG_MODE_SEL_REG_OFFSET) , Port_ConfigPtr[Pin].pin_num);    /* Clear the corresponding bit in the GPIOAMSEL register to disable analog functionality on this pin */
    
      SET_BIT(*(volatile uint32 *)((volatile uint8 *)PORT_BaseAddressPtr + PORT_ALT_FUNC_REG_OFFSET) , Port_ConfigPtr[Pin].pin_num);             /* Enable Alternative function for this pin by clear the corresponding bit in GPIOAFSEL register */
  
      *(volatile uint32 *)((volatile uint8 *)PORT_BaseAddressPtr + PORT_CTL_REG_OFFSET) |= (0x0000000F & Port_ConfigPtr[Pin].pin_mode << (Port_ConfigPtr[Pin].pin_num * 4));     /* Set the PMCx bits for this pin */

      SET_BIT(*(volatile uint32 *)((volatile uint8 *)PORT_BaseAddressPtr + PORT_DIGITAL_ENABLE_REG_OFFSET) , Port_ConfigPtr[Pin].pin_num);       /* Set the corresponding bit in the GPIODEN register to enable digital functionality on this pin */
    }
#endif
}