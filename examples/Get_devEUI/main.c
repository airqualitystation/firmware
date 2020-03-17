/************************************************************************
 
 Ce programme permet d'avoir le devEUI de sa carte.
 Attention ce programme n'est pas compatible avec toute les cartes.
 Carte compatible : b-l072z-lrwan1

 Pour les autres cartes, essayez d'executer ce programme mais il est
 possible que celui ci retourne une erreur.
 N'oubliez pas de préciser la board que vous utilisez dans le fichier
 Makefile
 
*************************************************************************/

#include <stdio.h>

#define         ID1                                 ( 0x1FF80050 )
#define         ID2                                 ( 0x1FF80054 )
#define         ID3                                 ( 0x1FF80064 )

#define LORAMAC_DEVEUI_LEN             (8U)

void HW_GetUniqueId(uint8_t *id)
{
  id[7] = ((*(uint32_t *)ID1) + (*(uint32_t *)ID3)) >> 24;
  id[6] = ((*(uint32_t *)ID1) + (*(uint32_t *)ID3)) >> 16;
  id[5] = ((*(uint32_t *)ID1) + (*(uint32_t *)ID3)) >> 8;
  id[4] = ((*(uint32_t *)ID1) + (*(uint32_t *)ID3));
  id[3] = ((*(uint32_t *)ID2)) >> 24;
  id[2] = ((*(uint32_t *)ID2)) >> 16;
  id[1] = ((*(uint32_t *)ID2)) >> 8;
  id[0] = ((*(uint32_t *)ID2));
}

int main(void)
{
    
    uint8_t test_deveui[LORAMAC_DEVEUI_LEN];

    HW_GetUniqueId(test_deveui);
    printf("\n+--------+-------------------------+\n");
    printf("| devEUI | ");
    for(uint8_t i1=0;i1<LORAMAC_DEVEUI_LEN;i1++)
    {
    	printf("%x ",test_deveui[i1]);
    }
    printf("|\n+--------+-------------------------+\n");
    printf("\nYou are running RIOT on a(n) %s board.\n", RIOT_BOARD);
    printf("This board features a(n) %s MCU.\n", RIOT_MCU);

    return 0;
}
