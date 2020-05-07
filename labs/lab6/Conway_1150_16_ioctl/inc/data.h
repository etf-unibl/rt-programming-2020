#ifndef DATA_H
#define DATA_H

/* Struktura koja opisuje rad simulacije, statisticke podatke! */
typedef struct{
	int broj_zivih_celija;
	int broj_rodjenih;
	int broj_umrlih;
	int broj_generacija;
}STATISTIKA;

#define dev_file "/dev/etx_device"

#endif
