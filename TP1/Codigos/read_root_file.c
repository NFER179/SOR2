#include <stdio.h>
#include <string.h>
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
    char volume_id[4];
    char volume_label[11];
    char fs_type[8];
    char boot_code[448];
    unsigned short boot_sector_signature;
} __attribute((packed)) Fat12BootSector;

typedef struct {
    unsigned char filename[8];
    unsigned char fileExtension[3];
    unsigned char fileAtributes;
    unsigned char reserved;
    unsigned char createdTime;
    unsigned char createdHour[2];
    unsigned char createdDay[2];
    unsigned char accessedDay[2];
    unsigned char highBytesOfFirstClusterAddress[2];
    unsigned char writenTime[2];
    unsigned char writenDay[2];
    unsigned short lowBytesOfFirstClusterAddress;
    unsigned int sizeOfFile;
} __attribute((packed)) Fat12Entry;

void writeFileInfo(FILE * in, FILE * out,
		Fat12Entry *entry,
		unsigned int fatStart,
		unsigned int dataStart,
		unsigned int clusterSize, 
		unsigned int fileSize,
		unsigned int nextFat12Entry) {

	unsigned short cluster = entry->lowBytesOfFirstClusterAddress;
	unsigned char buffer[4096];
	size_t bytesRead, bytesToRead, fileS = fileSize, clusterS = clusterSize;

	/* localizo el bloque de datos. */
	unsigned int block = dataStart + ( clusterS * (cluster - 2));

	switch(entry->filename[0]) {
		case 0x00:
			return;
		case 0xE5:
			return;
		case 0x05:
			return;
		case 0x2E:
			return;
		default:
			switch(entry->fileAtributes) {
				case 0x10: //Es directorio.
					fwrite("Directorio: ", sizeof("Directorio:"), 1, out);
					fwrite(entry->filename, sizeof(entry->filename), 1, out);
					fwrite("\n", sizeof(char), 1, out);
					return;
				case 0x20: //En el caso de que sea archivo.
					printf("%.8s.%.3s\n", entry->filename, entry->fileExtension);
					fwrite("Nombre del Archivo: ", sizeof("Nombre del Archivo:"), 1, out);
					fwrite(entry->filename, sizeof(entry->filename), 1, out);
					fwrite(".", sizeof(char), 1, out);
					fwrite(entry->fileExtension, sizeof(entry->fileExtension), 1, out);
					fwrite("\n", sizeof(char), 1, out);
					
					// Go to first data cluster
					fseek(in, block, SEEK_SET);

					// Read until we run out of file or clusters
					while(fileS > 0 && cluster != 0xffff) {
//printf("%d\n", block);
						bytesToRead = sizeof(buffer);

						// don't read past the file or cluster end
					        if(bytesToRead > fileS)
					            bytesToRead = fileS;
					        if(bytesToRead > clusterS)
					            bytesToRead = clusterS;

						// read data from cluster, write to file
						bytesRead = fread(buffer, 1, bytesToRead, in);
						fwrite(buffer, 1, bytesRead, out);
						//        printf("Copied %d bytes\n", bytes_read);

						// decrease byte counters for current cluster and whole file
						clusterS -= bytesRead;
						fileS -= bytesRead;

						// if we have read the whole cluster, read next cluster # from FAT
						if(clusterS == 0) {
							fseek(in, fatStart + cluster*2, SEEK_SET);
							fread(&cluster, 2, 1, in);

							//printf("End of cluster reached, next cluster %d\n", cluster);

							fseek(in, dataStart + clusterS * (cluster-2), SEEK_SET);
							clusterS = clusterSize; // reset cluster byte counter
						}						
					};

					/* posiciona el puntero para leer la siguiente root entry */
					fseek(in, nextFat12Entry, SEEK_SET);

					return;
				default:
					return;
			}
	}
}

int main() {

	FILE * in = fopen("test.img", "rb");
	FILE *out;

	int i;

	PartitionTable pt[4];

	Fat12BootSector bs;

	Fat12Entry entry;

	/* Creamos directorio para dejar los archivos leidos. */
	mkdir("leidos", 0777);

	fseek(in, 0x1BE, SEEK_SET);//Ir al inicioa de la tabla de particiones.

	fread(pt, sizeof(PartitionTable), 4, in); //Leo entradas.

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

	fread(&bs, sizeof(Fat12BootSector), 1, in); //&bs (pointer) direccion de bs;

	/* Obtengo el posicionamiento del inicio de root enrty */
	unsigned int fatStart = ftell(in) + (bs.reserved_sectors - 1) * bs.sector_size;
	unsigned int rootStart = fatStart + bs.fat_size_sectors * bs.number_of_fats * bs.sector_size;
	unsigned int dataStart = rootStart + (bs.root_dir_entries * sizeof(entry));

	fseek(in, rootStart, SEEK_SET);

	/* Busqueda de archivos. */
	out = fopen("leidos/archivosLeidos.txt", "w+");

	for(i=0; i<bs.root_dir_entries; i++) {

		fread(&entry, sizeof(entry), 1, in);

		writeFileInfo(in, out, &entry, 
			fatStart,
			dataStart, 
			bs.SectorsPerCluster * bs.sector_size,
			entry.sizeOfFile,
			rootStart + (sizeof(entry) * (i + 1) ));

	}

	fclose(out);
	fclose(in);
	return 0;
}
