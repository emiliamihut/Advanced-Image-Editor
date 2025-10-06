#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef struct {
	int r, g, b; // componente RGB
	int l;       // grayscale-luminozitate
} pixel;
typedef struct {
	pixel **pixels;  // matricea de pixeli
	int width;
	int height;
	int max_val;
	char format[3]; // formatul imaginii
	int x1, y1, x2, y2; // coordonatele zonei selectate
} image;

int clamp(int value)
{
	if (value < 0)
		return 0;
	if (value > 255)
		return 255;
	return value;
}

void free_img(image *img)
{
	if (img) {
		for (int i = 0; i < (*img).height; i++) {
			free((*img).pixels[i]);
		}
		free((*img).pixels);
		free(img);
	}
}

image *load_img(char *path)
{
	char ch;
	FILE *file = fopen(path, "rb");
	if (!file) {
		printf("Failed to load %s\n", path);
		return NULL;
	}
	image *img = calloc(1, sizeof(image));
	fscanf(file, "%s", (*img).format); //P2,P3,P5,P6
	fscanf(file, "%d %d", &(*img).width, &(*img).height);
	fscanf(file, "%d", &(*img).max_val);
	fscanf(file, "%c", &ch);

	(*img).pixels = calloc((*img).height, sizeof(pixel *));
	for (int i = 0; i < (*img).height; i++) {
		(*img).pixels[i] = calloc((*img).width, sizeof(pixel));
	}
	// matricele in functie de format
	if (strcmp((*img).format, "P2") == 0) {
		// Grayscale ASCII
		for (int i = 0; i < (*img).height; i++) {
			for (int j = 0; j < (*img).width; j++) {
				fscanf(file, "%d", &(*img).pixels[i][j].l);
			}
		}
	} else if (strcmp((*img).format, "P3") == 0) {
		// Color ASCII
		for (int i = 0; i < (*img).height; i++) {
			for (int j = 0; j < (*img).width; j++) {
				fscanf(file, "%d", &(*img).pixels[i][j].r);
				fscanf(file, "%d", &(*img).pixels[i][j].g);
				fscanf(file, "%d", &(*img).pixels[i][j].b);
			}
		}
	} else if (strcmp((*img).format, "P5") == 0) {
		// Grayscale binar
		for (int i = 0; i < (*img).height; i++) {
			for (int j = 0; j < (*img).width; j++) {
				unsigned char gray;
				fread(&gray, sizeof(unsigned char), 1, file);
				(*img).pixels[i][j].l = gray;
			}
		}
	} else if (strcmp((*img).format, "P6") == 0) {
		// Color binar
		for (int i = 0; i < (*img).height; i++) {
			for (int j = 0; j < (*img).width; j++) {
				unsigned char r, g, b;
				fread(&r, sizeof(unsigned char), 1, file);
				fread(&g, sizeof(unsigned char), 1, file);
				fread(&b, sizeof(unsigned char), 1, file);
				(*img).pixels[i][j].r = r;
				(*img).pixels[i][j].g = g;
				(*img).pixels[i][j].b = b;
			}
		}
	}
	// selectam intreaga imagine
	(*img).x1 = 0;
	(*img).y1 = 0;
	(*img).x2 = (*img).width;
	(*img).y2 = (*img).height;

	printf("Loaded %s\n", path);
	fclose(file);
	return img;
}

void exit_program(image *img)
{
	if (!img)
		printf("No image loaded\n");
	else
		free_img(img);
}

void select_area(image *img, int x1, int y1, int x2, int y2)
{
	if (!img) {
		printf("No image loaded\n");
		return;
	}
	//verificam ordinea
	int aux;
	if (x1 > x2) {
		aux = x1;
		x1 = x2;
		x2 = aux;
	}
	if (y1 > y2) {
		aux = y1;
		y1 = y2;
		y2 = aux;
	}
	// verificare coordonate valide
	if (x1 < 0 || x2 > (*img).width || y1 < 0 || y2 > (*img).height ||
		x2 == x1 || y2 == y1) {
		printf("Invalid set of coordinates\n");
		return;
	}
	// actualizam
	(*img).x1 = x1;
	(*img).y1 = y1;
	(*img).x2 = x2;
	(*img).y2 = y2;

	printf("Selected %d %d %d %d\n", x1, y1, x2, y2);
}

void select_all(image *img)
{
	if (!img) {
		printf("No image loaded\n");
		return;
	}

	(*img).x1 = 0;
	(*img).y1 = 0;
	(*img).x2 = (*img).width;
	(*img).y2 = (*img).height;

	printf("Selected ALL\n");
}

void crop(image *img)
{
	if (!img) {
		printf("No image loaded\n");
		return;
	}
	// noile dimensiuni
	int new_w = (*img).x2 - (*img).x1;
	int new_h = (*img).y2 - (*img).y1;

	pixel **new_pixels = calloc(new_h, sizeof(pixel *));
	for (int i = 0; i < new_h; i++) {
		new_pixels[i] = calloc(new_w, sizeof(pixel));
	}
	// matricea cropped
	for (int i = 0; i < new_h; i++) {
		for (int j = 0; j < new_w; j++) {
			new_pixels[i][j] = (*img).pixels[(*img).y1 + i][(*img).x1 + j];
		}
	}
	// eliberam matricea veche
	for (int i = 0; i < (*img).height; i++) {
		free((*img).pixels[i]);
	}
	free((*img).pixels);
	// inlocuim cu noile valori
	(*img).pixels = new_pixels;
	(*img).width = new_w;
	(*img).height = new_h;
	// noile coordonate ale zonei de selectie
	(*img).x1 = 0;
	(*img).y1 = 0;
	(*img).x2 = new_w;
	(*img).y2 = new_h;
	printf("Image cropped\n");
}

void rotate_full(image *img, pixel **new_pixels, int new_w, int new_h, int a)
{
	for (int i = 0; i < new_h; i++) {
		for (int j = 0; j < new_w; j++) {
			if (a == 90) {
				new_pixels[i][j] =
				(*img).pixels[(*img).height - j - 1][i];
			} else if (a == 180) {
				new_pixels[i][j] =
				(*img).pixels[(*img).height - i - 1][(*img).width - j - 1];
			} else if (a == 270) {
				new_pixels[i][j] =
				(*img).pixels[j][(*img).width - i - 1];
			}
		}
	}
}

void rotate_select(image *img, pixel **new_pixels, int new_w, int new_h, int a)
{
	for (int i = 0; i < new_h; i++) {
		for (int j = 0; j < new_w; j++) {
			if (a == 90) {
				new_pixels[i][j] =
				(*img).pixels[(*img).y1 + new_w - j - 1][(*img).x1 + i];
			} else if (a == 180) {
				new_pixels[i][j] = (*img).pixels[(*img).y1 + new_h - i - 1]
				[(*img).x1 + new_w - j - 1];
			} else if (a == 270) {
				new_pixels[i][j] =
				(*img).pixels[(*img).y1 + j][(*img).x2 - i - 1];
			}
		}
	}
}

void rotate(image *img, int angle)
{
	int copy = angle;
	if (!img) {
		printf("No image loaded\n");
		return;
	}
	// multiplu de 90
	if (angle % 90 != 0) {
		printf("Unsupported rotation angle\n");
		return;
	}
	// interval [0, 360]
	while (angle < 0)
		angle += 360;
	angle %= 360;
	// verificare zona = intreaga imagine
	int full = ((*img).x1 == 0 && (*img).y1 == 0 &&
				(*img).x2 == (*img).width && (*img).y2 == (*img).height);
	// verificare square
	if (!full && ((*img).x2 - (*img).x1) != ((*img).y2 - (*img).y1)) {
		printf("The selection must be square\n");
		return;
	}
	if (angle == 0) { // nu se schimba
		printf("Rotated %d\n", copy);
		return;
	}
	int new_w = 0, new_h = 0;// new width si new height
	// calculam dimensiunile noii imagini
	if (full) {
		if (angle == 90 || angle == 270) { // se inverseaza
			new_w = (*img).height;
			new_h = (*img).width;
		} else { // pentru 180
			new_w = (*img).width;
			new_h = (*img).height;
		}
	} else { // dimensiunile zonei selectate
		new_w = (*img).x2 - (*img).x1;
		new_h = (*img).y2 - (*img).y1;
	}
	pixel **new_pixels = calloc(new_h, sizeof(pixel *));
	for (int i = 0; i < new_h; i++) {
		new_pixels[i] = calloc(new_w, sizeof(pixel));
	}
	// aplicam rotatia in functie de unghi
	if (full) {
		rotate_full(img, new_pixels, new_w, new_h, angle);
	} else {
		rotate_select(img, new_pixels, new_w, new_h, angle);
	}
	// revenim la imaginea initiala
	if (full) {
		for (int i = 0; i < (*img).height; i++) {
			free((*img).pixels[i]);
		}
		free((*img).pixels);
		(*img).pixels = new_pixels;
		(*img).width = new_w;
		(*img).height = new_h;
	} else { // actualizam doar zona selectata
		for (int i = 0; i < new_h; i++) {
			for (int j = 0; j < new_w; j++) {
				(*img).pixels[(*img).y1 + i][(*img).x1 + j] =
				new_pixels[i][j];
			}
			free(new_pixels[i]);
		}
		free(new_pixels);
	}
	// actualizam coordonatele zonei selectate
	if (full) {
		(*img).x1 = 0;
		(*img).y1 = 0;
		(*img).x2 = new_w;
		(*img).y2 = new_h;
	}
	printf("Rotated %d\n", copy);
}

void save_image(image *img, char *filename, int ascii)
{
	if (!img) {
		printf("No image loaded\n");
		return;
	}
	FILE *fisier = fopen(filename, "wb");
	// header
	if (ascii) {
		if (!strcmp((*img).format, "P2") || !strcmp((*img).format, "P5")) {
			fprintf(fisier, "P2\n"); // Grayscale ASCII
		} else {
			fprintf(fisier, "P3\n"); // Color ASCII
		}
	} else {
		if (!strcmp((*img).format, "P2") || !strcmp((*img).format, "P5")) {
			fprintf(fisier, "P5\n"); // Grayscale Binary
		} else {
			fprintf(fisier, "P6\n"); // Color Binary
		}
	}
	fprintf(fisier, "%d %d\n", (*img).width, (*img).height);
	fprintf(fisier, "%d\n", (*img).max_val);
	// pixelii
	for (int i = 0; i < (*img).height; i++) {
		for (int j = 0; j < (*img).width; j++) {
			if (ascii) {
				// ASCII
				if (!strcmp((*img).format, "P2") ||
					!strcmp((*img).format, "P5")) {
					fprintf(fisier, "%d ", (*img).pixels[i][j].l);
				} else {
					fprintf(fisier, "%d ", (*img).pixels[i][j].r);
					fprintf(fisier, "%d ", (*img).pixels[i][j].g);
					fprintf(fisier, "%d ", (*img).pixels[i][j].b);
				}
			} else { // binar
				if (!strcmp((*img).format, "P2") ||
					!strcmp((*img).format, "P5")) {
					unsigned char gray = (unsigned char)(*img).pixels[i][j].l;
					fwrite(&gray, sizeof(unsigned char), 1, fisier);
				} else {
					unsigned char r = (unsigned char)(*img).pixels[i][j].r;
					unsigned char g = (unsigned char)(*img).pixels[i][j].g;
					unsigned char b = (unsigned char)(*img).pixels[i][j].b;
					fwrite(&r, sizeof(unsigned char), 1, fisier);
					fwrite(&g, sizeof(unsigned char), 1, fisier);
					fwrite(&b, sizeof(unsigned char), 1, fisier);
				}
			}
		}
		if (ascii) {
			fprintf(fisier, "\n"); // pt ASCII
		}
	}
	fclose(fisier);
	printf("Saved %s\n", filename);
}

int ker_and_div(char param[], int kernel[3][3])
{
	int divisor = 1; // implicit 1

	if (strcmp(param, "EDGE") == 0) {
		int aux[3][3] = {
			{-1, -1, -1},
			{-1,  8, -1},
			{-1, -1, -1}
		};
		memcpy(kernel, aux, sizeof(aux));
	} else if (strcmp(param, "SHARPEN") == 0) {
		int aux[3][3] = {
			{ 0, -1,  0},
			{-1,  5, -1},
			{ 0, -1,  0}
		};
		memcpy(kernel, aux, sizeof(aux));
	} else if (strcmp(param, "BLUR") == 0) {
		int aux[3][3] = {
			{1, 1, 1},
			{1, 1, 1},
			{1, 1, 1}
		};
		memcpy(kernel, aux, sizeof(aux));
		divisor = 9;
	} else if (strcmp(param, "GAUSSIAN_BLUR") == 0) {
		int aux[3][3] = {
			{1, 2, 1},
			{2, 4, 2},
			{1, 2, 1}
		};
		memcpy(kernel, aux, sizeof(aux));
		divisor = 16;
	}

	return divisor;
}

void apply(image *img, char param[])
{
	// vectori pentru vecini
	int dx[9] = {-1, -1, -1, 0, 0, 0, 1, 1, 1};
	int dy[9] = {-1,  0,  1, -1, 0, 1, -1, 0, 1};

	if (!img) {
		printf("No image loaded\n");
		return;
	}
	// daca nu avem parametru
	if (strcmp(param, "zero") == 0) {
		printf("Invalid command\n");
		return;
	}
	// daca parametrul e diferit de cele prestabilite
	if (strcmp(param, "EDGE") && strcmp(param, "SHARPEN") &&
		strcmp(param, "BLUR") && strcmp(param, "GAUSSIAN_BLUR")) {
		printf("APPLY parameter invalid\n");
		return;
	}
	// verificare imagine color
	if (strcmp((*img).format, "P3") != 0 && strcmp((*img).format, "P6") != 0) {
		printf("Easy, Charlie Chaplin\n");
		return;
	}
	// matricele kernel in functie de parametru
	int kernel[3][3];
	int divisor = ker_and_div(param, kernel);
	// matricea copie pentru pixelii modificati
	pixel **new_pixels = calloc((*img).height, sizeof(pixel *));
	for (int i = 0; i < (*img).height; i++) {
		new_pixels[i] = calloc((*img).width, sizeof(pixel));
		for (int j = 0; j < (*img).width; j++) {
			new_pixels[i][j] = (*img).pixels[i][j];
		}
	}
	// aplicare kernel pe selectie
	for (int i = (*img).y1 ; i < (*img).y2 ; i++) {
		for (int j = (*img).x1 ; j < (*img).x2 ; j++) {
			// pe margine
			if (i != 0 && i != (*img).height - 1 && j != 0 &&
				j != (*img).width - 1) {
				double new_r = 0, new_g = 0, new_b = 0;
				// aplicare kernel
				for (int k = 0; k < 9; k++) {
					int ii = i + dy[k];
					int jj = j + dx[k];
					new_r += kernel[k / 3][k % 3] * (*img).pixels[ii][jj].r;
					new_g += kernel[k / 3][k % 3] * (*img).pixels[ii][jj].g;
					new_b += kernel[k / 3][k % 3] * (*img).pixels[ii][jj].b;
				}
				// aplicam divizorul si clamp
				new_pixels[i][j].r = clamp((int)(new_r / divisor));
				new_pixels[i][j].g = clamp((int)(new_g / divisor));
				new_pixels[i][j].b = clamp((int)(new_b / divisor));
			}
		}
	}
	// eliberam matricea veche de pixeli
	for (int i = 0; i < (*img).height; i++) {
		free((*img).pixels[i]);
	}
	free((*img).pixels);
	// actualizam matricea cu noile valori
	(*img).pixels = new_pixels;
	printf("APPLY %s done\n", param);
}

void histogram(image *img, int x, int y)
{
	if (!img) {
		printf("No image loaded\n");
		return;
	}
	// verificre alb-negru
	if (strcmp((*img).format, "P2") && strcmp((*img).format, "P5")) {
		printf("Black and white image needed\n");
		return;
	}
	// verificare y putere a lui 2 (pe biti) și interval
	if (y < 2 || y > 256 || (y & (y - 1)) != 0) {
		printf("Invalid set of parameters\n");
		return;
	}
	int *histogram = calloc(y, sizeof(int));

	int size = ((*img).max_val + 1) / y; // 255 + 1 e nr maxim de valori

	for (int i = 0; i < (*img).height; i++) {
		for (int j = 0; j < (*img).width; j++) {
			int nr = (*img).pixels[i][j].l / size; // in care bin apare
			histogram[nr]++;
		}
	}
	// frecventa maxima
	int max = 0;
	for (int i = 0; i < y; i++) {
		if (histogram[i] > max) {
			max = histogram[i];
		}
	}
	// generare
	for (int i = 0; i < y; i++) {
		int nr = (int)((double)histogram[i] / max * x);
		printf("%d\t|\t", nr);
		for (int j = 0; j < nr; j++) {
			printf("*");
		}
		printf("\n");
	}
	// eliberam memoria
	free(histogram);
}

void equalize(image *img)
{
	if (!img) {
		printf("No image loaded\n");
		return;
	}
	// verificare alb-negru
	if (strcmp((*img).format, "P2") && strcmp((*img).format, "P5")) {
		printf("Black and white image needed\n");
		return;
	}
	// calcul histograma
	int hist[256] = {0}; // initializam cu 0 fiecre valoare posibila
	for (int i = 0; i < (*img).height; i++)
		for (int j = 0; j < (*img).width; j++)
			hist[(*img).pixels[i][j].l]++;

	int area = (*img).width * (*img).height; // aria
	int sum = 0; // suma frecventelor pana la a (val curenta)
	int new_value[256]; // noua valoare
	for (int i = 0; i < 256; i++) {
		sum += hist[i]; // adaugam frecventa curenta la suma
		new_value[i] = (int)round(((double)sum / area) * 255);// formula
		new_value[i] = clamp(new_value[i]);
	}
	// aplicam pe toti pixelii
	for (int i = 0; i < (*img).height; i++)
		for (int j = 0; j < (*img).width; j++)
			(*img).pixels[i][j].l = new_value[(*img).pixels[i][j].l]; //f(a)

	printf("Equalize done\n");
}

int main(void)
{
	char command[10], file_path[100], type[10], param[30], c;
	image *current_img = NULL;
	int over = 0, angle, x, y, x1, x2, y1, y2;
	while (scanf("%s", command) && over == 0) {
		if (strcmp(command, "LOAD") == 0) {
			scanf("%s", file_path);
			if (current_img) // eliberam imaginea anterioara
				free_img(current_img);
			current_img = load_img(file_path);
		} else if (strcmp(command, "SELECT") == 0) {
			if (fgets(param, sizeof(param), stdin)) { // rest
				if (!current_img) {
					printf("No image loaded\n");
					continue;
				}
				if (sscanf(param, "%s", type) == 1 &&
					strcmp(type, "ALL") == 0) {
					select_all(current_img);
				} else if (sscanf(param, "%d %d %d %d %c", &x1, &y1, &x2, &y2,
				 &c) == 4) {
					select_area(current_img, x1, y1, x2, y2);
				} else {
					printf("Invalid command\n");
					continue;
				}
			}
		} else if (strcmp(command, "ROTATE") == 0) {
			scanf("%d", &angle);
			rotate(current_img, angle);
		} else if (strcmp(command, "CROP") == 0) {
			crop(current_img);
		} else if (strcmp(command, "SAVE") == 0) {
			int ascii = 0; // 1/0 ascii/binar
			scanf("%s", file_path);
			if (getchar() == ' ')
				if (getchar() != '\n') { // parametru suplimentar
					scanf("%s", param);
					if (strcmp(param, "ascii") == 0)
						ascii = 1;
				}
			save_image(current_img, file_path, ascii);
		} else if (strcmp(command, "APPLY") == 0) {
			if (getchar() == ' ') {
				scanf("%s", param);
				apply(current_img, param);
			} else
				apply(current_img, "zero"); // fara parametru
		} else if (strcmp(command, "HISTOGRAM") == 0) {
			if (fgets(param, sizeof(param), stdin)) { // rest
				if (!current_img) {
					printf("No image loaded\n");
					continue;
				}
				if (sscanf(param, "%d %d %c", &x, &y, &c) == 2)
					histogram(current_img, x, y);
				else {
					printf("Invalid command\n");
					continue;
				}
			}
		} else if (strcmp(command, "EQUALIZE") == 0) {
			equalize(current_img);
		} else if (strcmp(command, "EXIT") == 0) {
			exit_program(current_img);
			over = 1;
		} else {
			printf("Invalid command\n");
			// consumam restul liniei
			while (getchar() != '\n') {

			}
		}
	}
	return 0;
}
