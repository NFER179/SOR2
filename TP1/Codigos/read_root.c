#include <stdio.h>

#include <stdlib.h>



typedef struct {

    unsigned char first_byte;

    unsigned char start_chs[3];

    unsigned char partition_type;

    unsigned char end_chs[3];

    unsigned short starting_cluster;

    unsigned int file_size;

} __attribute((packed)) PartitionTable;



typedef struct {

    unsigned char jmp[3];

    char oem[8];

    unsigned short sector_size;

	// -> Inicio, completando struct

    unsigned char SectorsPerCluster;

    unsigned short reserved_sectors;

    unsigned char number_of_fats;

    unsigned short root_dir_entries;

    unsigned short totalLogicalSectors;

    unsigned char mediaDescriptor;

    unsigned short fat_size_sectors;

    unsigned short physicalSectorsPerTrack;

    unsigned short numberOfHead;

    unsigned char hiddenSectors[4];

    unsigned char totalLogicSector[4];

    unsigned char physicalDriveNumber;

    unsigned char reserved;

    unsigned char extendedBootSignature;



	// <- Fin

    char volume_id[4];

    char volume_label[11];

    char fs_type[8];

    char boot_code[448];

    unsigned short boot_sector_signature;

} __attribute((packed)) Fat12BootSector;



typedef struct {

	// -> Inicio

    unsigned char filename[8];

    unsigned char fileExtension[3];

    unsigned char fileAtrbutes;

    unsigned char reserved;

    unsigned char createdTime;

    unsigned char createdHour[2];

    unsigned char createdDay[2];

    unsigned char accessedDay[2];

    unsigned char highBytesOfFirstClusterAddress[2];

    unsigned char writenTime[2];

    unsigned char writenDay[2];

    unsigned short lowBytesOfFirstClusterAddress;

    unsigned char sizeOfFile[4];

	// <- Fin

} __attribute((packed)) Fat12Entry;



void print_file_info(Fat12Entry *entry) {

  
    switch(entry->filename[0]) {

    case 0x41:
        return; // unused entry  
      
    case 0x00:
        return; // unused entry

    case 0xE5:
	printf(" File Attribute [0x%X] ", entry->fileAtrbutes);
      
        printf("Deleted file: [%.7s.%.3s]\n", entry->filename, entry->fileExtension);

        return;

    case 0x05:

        printf("File starting with 0xE5: [%c%.7s.%.3s]\n", 0xE5, entry->filename, entry->fileExtension);

        break;

    case 0x2E:

        printf("SubDirectory: [%.8s.%.3s]\n", entry->filename, entry->fileExtension);

        break;

    default:


	 if(entry->fileAtrbutes == 0x10){
	      printf(" File Attribute [0x%X] ", entry->fileAtrbutes);
	      printf("Directory: [%.8s.%.3s]\n", entry->filename, entry->fileExtension);
	      
	        
	 }
	 else if (entry->fileAtrbutes == 0x20){
	      printf(" File Attribute [0x%X] ", entry->fileAtrbutes);
	      printf("File: [%.8s.%.3s]\n", entry->filename, entry->fileExtension);
	 }
    }



}



int main() {

    FILE * in = fopen("test.img", "rb");

    int i;

    PartitionTable pt[4];

    Fat12BootSector bs;

    Fat12Entry entry;

	// -> Inicio

    fseek(in, 0x1BE, SEEK_SET);//Ir al inicioa de la tabla de particiones.

    fread(pt, sizeof(PartitionTable), 4, in); //Leo entradas.

	// <- Fin



    for(i=0; i<4; i++) {

        if(pt[i].partition_type == 1) {

            printf("Encontrada particion FAT12 %d\n", i);

            break;

        }

    }



    if(i == 4) {

        printf("No encontrado filesystem FAT12, saliendo...\n");

        return -1;

    }



    fseek(in, 0, SEEK_SET);

	//{...} Leo boot sector

	//-> Inicio

//&bs (pointer) direccion de bs;

    fread(&bs, sizeof(Fat12BootSector), 1, in);

	//<- Fin



    printf("En  0x%X, sector size %d, FAT size %d sectors, %d FATs\n\n",

           (unsigned int)ftell(in), bs.sector_size, bs.fat_size_sectors, bs.number_of_fats);



    fseek(in, (bs.reserved_sectors-1 + bs.fat_size_sectors * bs.number_of_fats) *

          bs.sector_size, SEEK_CUR);



    printf("Root dir_entries %d \n", bs.root_dir_entries);

    for(i=0; i<bs.root_dir_entries; i++) {

        fread(&entry, sizeof(entry), 1, in);

        print_file_info(&entry);

    }



    printf("\nLeido Root directory, ahora en 0x%X\n", (unsigned int)ftell(in));

    //leo dataArea.
    
        printf("En  0x%X, sector size %d, FAT size %d sectors, %d FATs\n\n",

           (unsigned int)ftell(in), bs.sector_size, bs.fat_size_sectors, bs.number_of_fats);



    fseek(in, (bs.reserved_sectors-1 + bs.fat_size_sectors * bs.number_of_fats) *

          bs.sector_size, SEEK_CUR);



    printf("Root dir_entries %d \n", bs.root_dir_entries);

    for(i=0; i<bs.root_dir_entries; i++) {

        fread(&entry, sizeof(entry), 1, in);

        print_file_info(&entry);

    }



    printf("\nLeido Root directory, ahora en 0x%X\n", (unsigned int)ftell(in));
    
    fclose(in);

    return 0;

}


