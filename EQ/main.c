//////////////////////////////////////////////////////////////////////////////
// *
// * Predmetni projekat iz predmeta OAiS DSP 2
// * Godina: 2017
// *
// * Zadatak: Ekvalizacija audio signala
// * Autor:
// *                                                                          
// *                                                                          
/////////////////////////////////////////////////////////////////////////////

#include "stdio.h"
#include "ezdsp5535.h"
#include "ezdsp5535_i2c.h"
#include "aic3204.h"
#include "ezdsp5535_aic3204_dma.h"
#include "ezdsp5535_i2s.h"
#include "ezdsp5535_sar.h"
#include "print_number.h"
#include "math.h"
#include "string.h"
#include "iir.h"
#include "processing.h"

/* Frekvencija odabiranja */
#define SAMPLE_RATE 16000L

#define PI 3.14159265

/* Niz za smestanje ulaznih i izlaznih odbiraka */
#pragma DATA_ALIGN(sampleBufferL,4)
Int16 sampleBufferL[AUDIO_IO_SIZE];
#pragma DATA_ALIGN(sampleBufferR,4)
Int16 sampleBufferR[AUDIO_IO_SIZE];

Int16 cLP[4];
Int16 cHP[4];

Int16 cPeek1[6];
Int16 cPeek2[6];

Int16 x_hLP = 0;
Int16 y_hLP = 0;
Int16 x_hHP = 0;
Int16 y_hHP = 0;
Int16 x_hPeek1[2];
Int16 y_hPeek1[2];
Int16 x_hPeek2[2];
Int16 y_hPeek2[2];

Int16 output1[AUDIO_IO_SIZE];
Int16 output2[AUDIO_IO_SIZE];
Int16 output3[AUDIO_IO_SIZE];


void main( void )
{

    int i=0, filter=0;
    int k[4]={8192,8192,8192,8192};
    char ch;

    memset(cLP,0,4);
    memset(cHP,0,4);

    memset(cPeek1, 0, 6);
    memset(cPeek2, 0, 6);

    memset(x_hPeek1, 0, 2);
    memset(x_hPeek2, 0, 2);
    memset(y_hPeek1, 0, 2);
    memset(y_hPeek2, 0, 2);

    int dirac_impulse;

    Uint16 mySW = NoKey;

    /* Inicijalizaija razvojne ploce */
    EZDSP5535_init( );

    /* Inicijalizacija kontrolera za ocitavanje vrednosti pritisnutog dugmeta*/
    EZDSP5535_SAR_init();

    /* Inicijalizacija LCD kontrolera */
    initPrintNumber();

	printf("\n Ekvalizacija audio signala \n");

    /* Inicijalizacija veze sa AIC3204 kodekom (AD/DA) */
    aic3204_hardware_init();

    /* Inicijalizacija AIC3204 kodeka */
	aic3204_init();

    aic3204_dma_init();

    /* Postavljanje vrednosti frekvencije odabiranja i pojacanja na kodeku */
    set_sampling_frequency_and_gain(SAMPLE_RATE, 0);

    while(1)
    {
    	aic3204_read_block(sampleBufferL, sampleBufferR);

    	/* Your code here */
        mySW = EZDSP5535_SAR_getKey();

        if (mySW==SW1) {
            filter++;
            if (filter>3) filter=0;
            clearLCD();

            if (filter==0) {
                printChar('L');
                printChar('P');
            }
            else if (filter==1 || filter==2) {
                printChar('P');
                printChar('E');
                printChar('E');
                printChar('K');
            }
            else {
                printChar('H');
                printChar('P');
            }

            if ((int)k[filter]/10000!=0) {
                ch=(int)(k[filter]/10000)+'0';
                printChar(ch);
            }
            ch=(int)(k[filter]%10000)/1000+'0';
            printChar(ch);
            ch=(int)(k[filter]%1000)/100+'0';
            printChar(ch);
            ch=(int)(k[filter]%100)/10+'0';
            printChar(ch);
            ch=(int)(k[filter]%10)+'0';
            printChar(ch);
        } else if (mySW==SW2) {
            k[filter] -= 3277;
            if (k[filter]<0) {
                k[filter]=32767;
            }
        }
        else {

        }

//        calculateShelvingCoeff(0.3, cLP);
//        calculatePeekCoeff(0.7, 0, cPeek1);
//        calculateShelvingCoeff(-0.3, cHP);

        calculateShelvingCoeff(0.9243905, cLP);
        calculatePeekCoeff(0.8540807, 0.9645574, cPeek1);
        calculatePeekCoeff(0.6128009, 0.5877854, cPeek2);
        calculateShelvingCoeff(-0.9999993, cHP);

        for (i=0; i<AUDIO_IO_SIZE; i++) {
            if (i==0) dirac_impulse=16000;
            else dirac_impulse=0;

            output1[i]=shelvingLP(dirac_impulse, cLP, &x_hLP, &y_hLP, k[0]);
            output2[i]=shelvingPeek(output1[i], cPeek1, x_hPeek1, y_hPeek1, k[1]);
            output3[i]=shelvingPeek(output2[i], cPeek2, x_hPeek2, y_hPeek2,k[2]);
            sampleBufferR[i]=shelvingHP(output3[i], cHP, &x_hHP, &y_hHP,k[0]);
        }

		aic3204_write_block(sampleBufferR, sampleBufferR);
	}


	/* Prekid veze sa AIC3204 kodekom */
    aic3204_disable();

    printf( "\n***Kraj programa***\n" );
	SW_BREAKPOINT;
}


