#include <stdio.h>
#include <stdlib.h>

int main() {
    FILE * in = fopen("test.img", "rb");
    unsigned int i, start_sector, length_sectors;
    
    
    
    
    if (in == NULL)
        {
            printf("\nError de apertura del archivo. \n\n");
        }
        else
        { 
	  
	    printf("\n  ******************** 446 bytes (gestor de arranque) ******************* \n\n");
	  
	   int cantidadBytesParaLeer=446;
	   
	    for(int j=0; j<cantidadBytesParaLeer; j++) {
	      
	      i = fgetc(in);
          
	      printf("%02X", i);
		
	    }
	    
	   
	    printf("\n ******************************** Particiones: ******************************** \n\n");
	  
	  fseek(in, 0x1BE, SEEK_SET); // Voy al inicio... 
	  
	  for(i=1; i<5; i++) { // Leo las entradas (Particiones)

	      printf("Partition entry %d: First byte %02X\n", i, fgetc(in));
	      printf("  Partition start in CHS: %02X:%02X:%02X\n", fgetc(in), fgetc(in), fgetc(in));
	      printf("  Partition type 0x%02X\n", fgetc(in));
	      printf("  Partition end in CHS: %02X:%02X:%02X\n", fgetc(in), fgetc(in), fgetc(in));
	      
	      fread(&start_sector, 4, 1, in);
	      fread(&length_sectors, 4, 1, in);
	      printf("  Relative LBA address 0x%08X, %d sectors long\n", start_sector, length_sectors);
	      
	      printf("\n\n");
	      
	  }
	    printf("\n ****************************************************************************** \n\n");
	}
    fclose(in);
    return 0;
}