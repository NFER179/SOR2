#include <stdio.h>

#include <stdlib.h>



typedef struct {

    unsigned char first_byte;

    unsigned char start_chs[3];

    unsigned char partition_type;

    unsigned char end_chs[3];

    char start_sector[4];

    char length_sectors[4];

} __attribute((packed)) PartitionTable;



typedef struct {

    unsigned char jmp[3];

    char oem[8];

	/* inicio */

	/* 19 bytes - DOS 3.0 BPB */

	/** 13 bytes - DOS 2.0 BPB **/

	/*** 0x00B - primeros 2 bytes definidos por el profesor. ***/

    unsigned short sector_size; // 2 bytes

	// {...}  COMPLETAR

	/*** 0x00D - clusted size ***/

    unsigned char logicalSectorsPerCluster;

	/*** 0x00E - count of reservd logical sector***/

    unsigned char reservedLogicalSectors[2];

 	/*** 0x010 - FAT Number  ***/

    unsigned char numberOfFATs;

	/*** 0x011 - Maximun number of FAT root directory entries ***/

    unsigned char rootDirectoryEntries[2];

	/*** 0x013 - Total logical Sectors(if zero, use 4 byte value at offset) ***/

    unsigned char totalLogicalSectors[2];

	/*** 0x015 - Media descriptor  ***/

    unsigned char mediaDescriptor;

	/*** 0x016 - Logical Sector Per FAT ***/

    unsigned char logicalSectorPerFAT[2];

	/** 0x018 - Physical Sectors per Track **/

    unsigned char physicalSectorsPerTrack[2];

	/** 0x01A - Number of Head for Disk with INT 13h CHS geometry **/

    unsigned char numberOfHead[2];

	/** 0x01C - Count of Hidden Sectors preceding the partition that contains this FAT volume.  **/

    unsigned char hiddenSectors[4];

	/* 0x01E - Code Entry point for z80 processors into MSX boot code. */

	//    unsigned char codeEntryPointZ80[4];

	/* 0x020 - Total Logic Sector. */

    unsigned char totalLogicSector[4];

	/* 0x024 - Physical Drive Number. */

    unsigned char physicalDriveNumber;

	/* 0x025 - Reserved. */

    unsigned char reserved;

	/* 0x026 - Extended Boot Signature. */

    unsigned char extendedBootSignature;



	/* fin */



    char volume_id[4];

    char volume_label[11];

    char fs_type[8]; // Type in ascii

    char boot_code[448];

    unsigned short boot_sector_signature;

} __attribute((packed)) Fat12BootSector;



/* funcion que devuelve el string de offset 0x015 */

char* toTxt0x015(Fat12BootSector entry) {

	switch(entry.mediaDescriptor) {

	case 0xF8:

		return "Fixed Disk\n";

	default:

		return "No lo se Interpretar\n";

	}

}



int main() {

    FILE * in = fopen("test.img", "rb");

    int i;

    PartitionTable pt[4];

    Fat12BootSector bs;

    

    fseek(in, 0x1BE, SEEK_SET); // Ir al inicio de la tabla de particiones

    fread(pt, sizeof(PartitionTable), 4, in); // leo entradas 

    

    for(i=0; i<4; i++) {        

        printf("Partiion type: %d\n", pt[i].partition_type);

        if(pt[i].partition_type == 1) {

            printf("Encontrado FAT12 %d\n", i);

            break;

        }

    }

    

    if(i == 4) {

        printf("No FAT12 filesystem found, exiting...\n");

        return -1;

    }

    

    fseek(in, 0, SEEK_SET);

    fread(&bs, sizeof(Fat12BootSector), 1, in);

    

    printf("  0x000 - Jump code: %02X:%02X:%02X\n", bs.jmp[0], bs.jmp[1], bs.jmp[2]);

    printf("  0x003 - OEM code: [%.8s]\n", bs.oem);

    printf("  DOS 3.0 BPB:\n");

    printf("    DOS 2.0 BPB:\n");

    printf("      0x00B - sector_size: %d\n", bs.sector_size);

	// {...} COMPLETAR

	/* -> inicio */

    printf("      0x00D - Sectores por Cluster:                %d \n", bs.logicalSectorsPerCluster);

    printf("      0x00E - Sectores Reservados:                 %02X:%02X\n", bs.reservedLogicalSectors[0], bs.reservedLogicalSectors[1]);

    printf("      0x010 - Número de FAT's:                     %02X\n", bs.numberOfFATs);

    printf("      0x011 - Entradas Máximas en Directorio Raíz: %02X:%02X\n", bs.rootDirectoryEntries[0], bs.rootDirectoryEntries[1]);

    printf("      0x013 - Sectores Totales:                    %02X:%02X\n", bs.totalLogicalSectors[0], bs.totalLogicalSectors[1]);

    printf("      0x015 - Descripción de Medio:                %s", toTxt0x015(bs));

    printf("      0x016 - Sectores por FAT:                    %02X:%02X\n", bs.logicalSectorPerFAT[0], bs.logicalSectorPerFAT[1]);

    printf("    0x018 - Sectores por Pista:                  %02X:%02X\n", bs.physicalSectorsPerTrack[0], bs.physicalSectorsPerTrack[1]);

    printf("    0x01A - Número de Caras:                     %02X:%02X\n", bs.numberOfHead[0], bs.numberOfHead[1]);

    printf("    0x01C - Sectores Ocultos:                    %02X:%02X\n", bs.hiddenSectors[0], bs.hiddenSectors[1]);

//    printf("  0x01E - ?????                                %02X:%02X\n", bs.codeEntryPointZ80[0], bs.codeEntryPointZ80[1]);

    printf("  0x020 - Sectores Logicos Totales:            %02X:%02X:%02X:%02X\n", bs.totalLogicSector[0], bs.totalLogicSector[1], bs.totalLogicSector[3], bs.totalLogicSector[3]);

    printf("  0x024 - Número de unidad Fisica:             %02X\n", bs.physicalDriveNumber);

    printf("  0x025 - Reservado:                           %02X\n", bs.reserved);

    printf("  0x026 - Firma de arranque extendida:         %02X\n", bs.extendedBootSignature);



	/* Fin <- */

    printf("  volume_id: 0x%08X\n", (unsigned int)bs.volume_id);

    printf("  Volume label: [%.11s]\n", bs.volume_label);

    printf("  Filesystem type: [%.8s]\n", bs.fs_type);

    printf("  Boot sector signature: 0x%04X\n", bs.boot_sector_signature);

    

    fclose(in);

    return 0;

}



//Referencias

/* uso referencia de 'https://es.wikipedia.org/wiki/Tabla_de asignacion de archivos' */

/* wiki que paso el profe en la parte de boot sector. */