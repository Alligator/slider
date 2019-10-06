#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>

#define OPTPARSE_IMPLEMENTATION
#include "lib/optparse.h"
#include "lib/pdfgen.h"

// page width is dervied from the height based on the aspect ratio
#define PAGE_HEIGHT 540
#define FONT_SIZE 72

struct slide {
	bool monospace;
	int align;
	int font_size;
	char text[256];
};

void add_slide(struct pdf_doc *pdf, struct slide* sl, int page_width, int page_height, bool dark_mode) {
	struct pdf_object *page = pdf_append_page(pdf);

	pdf_page_set_size(pdf, page, page_width, page_height);
	pdf_set_font(pdf, sl->monospace ? "Courier-Bold" : "Helvetica-Bold");

	int colour;
	if (dark_mode) {
		colour = PDF_RGB(240, 240, 240);
		pdf_add_filled_rectangle(pdf, page, 0, 0, page_width, page_height, 0, PDF_BLACK);
	} else {
		colour = PDF_RGB(24, 24, 24);
		pdf_add_filled_rectangle(pdf, page, 0, 0, page_width, page_height, 0, PDF_RGB(240, 240, 240));
	}

	// measure text
	int height = pdf_add_text_wrap(pdf, NULL, sl->text, sl->font_size, 0, page_height / 2, colour, page_width, PDF_ALIGN_NO_WRITE);

	// write text centered
	int xoff = sl->align == PDF_ALIGN_LEFT ? sl->font_size : 0;
	int lines = height / sl->font_size;
	int yoff = page_height / 2 + ((lines - 1) * 0.5 * sl->font_size);

	pdf_add_text_wrap(pdf, NULL, sl->text, sl->font_size, xoff, yoff, colour, page_width, sl->align);
}

bool read_next_slide(FILE *fp, struct slide *sl) {
	// defaults
	sl->monospace = 0;
	sl->align = PDF_ALIGN_CENTER;
	sl->font_size = FONT_SIZE;

	int i = 0;
	int c;
	bool escape_mode = false;
	bool format_mode = true; // start by reading format characters

	while ((c = fgetc(fp)) != EOF) {
		// comments
		if (c == '#' && i == 0) {
			// eat the rest of the line and carry on
			while ((c = fgetc(fp)) != EOF) {
				if (c == '\n') {
					break;
				}
			}
			continue;
		}

		// format codes
		if (format_mode) {
			switch (c) {
				case '!': // monospace
					sl->monospace = true;
					break;
				case '<': // left align
					sl->align = PDF_ALIGN_LEFT;
					break;
				case '-': // smaller text
					sl->font_size = FONT_SIZE * 0.75;
					break;
				case ' ': // if we find a space, consume it and stop parsing format codes
					format_mode = false;
					c = fgetc(fp);
					break;
				default:
					format_mode = false;
					break;
			}

			if (format_mode)
				continue;
		}

		// escape characters
		if (escape_mode) {
			switch (c) {
				case 'n': c = '\n'; break;
				case 't': c = '\t'; break;
			}
			escape_mode = 0;
			sl->text[i++] = c;
			continue;
		}
		if (c == '\\') {
			escape_mode = 1;
			continue;
		}

		// check if the line has ended, or if we're about to overrun the buffer
		if (c == '\n' || i == sizeof(sl->text) - 1) {
			sl->text[i] = '\0';
			break;
		}

		// ignore carriage return
		if (c == '\r') {
			continue;
		}

		// if none of the above, write the character
		sl->text[i++] = c;
	}

	if (feof(fp))
		return false;

	return true;
}

void print_usage_and_exit() {
	printf(
		"usage: slider [options] file [output file]\n"
		"\n"
		"options:\n"
		"  -d	--dark	dark mode\n"
		"  -4	--4:3	render pages in 4:3 aspect ratio instead of 16:9 (the default)\n"
		"  -h	--help	show this message\n"
		"\n"
		"format codes:\n"
		"  !	monospace\n"
		"  <	left align\n"
		"  -	small font\n"
	);
	exit(0);
}

int main(int argc, char **argv) {
	// parse cmd line args
	struct optparse_long opts[] = {
		{ "dark", 'd', OPTPARSE_OPTIONAL },
		{ "help", 'h', OPTPARSE_OPTIONAL },
		{ "4:3", '4', OPTPARSE_OPTIONAL },
		{0},
	};

	bool dark_mode = false;
	int page_height = PAGE_HEIGHT;
	int page_width = PAGE_HEIGHT * 1.777; // 16:9 aspect ratio

	int option;
	struct optparse options;

	optparse_init(&options, argv);
	while ((option = optparse_long(&options, opts, NULL)) != -1) {
		switch (option) {
			case 'd':
				dark_mode = true;
				break;
			case '4':
				page_width = PAGE_HEIGHT * 1.333; // 4:3 aspect ratio
				break;
			case 'h':
				print_usage_and_exit();
			case '?':
				fprintf(stderr, "%s: %s\n", argv[0], options.errmsg);
				exit(1);
		}
	}

	char *file_name = optparse_arg(&options);
	if (file_name == NULL) {
		print_usage_and_exit();
	}

	char *output_file_name = optparse_arg(&options);
	if (output_file_name == NULL) {
		output_file_name = "output.pdf";
	}

	// open file
	FILE *fp = fopen(file_name, "r");
	if (fp == NULL) {
		fprintf(stderr, "can't open input file '%s'", file_name);
		exit(1);
	}

	// do the dang thing
	clock_t start = clock();

	struct pdf_info info = {
		.creator = "slider",
		.producer = "slider",
		.title = "presentation",
	};

	struct pdf_doc *pdf = pdf_create(PDF_A4_WIDTH, PDF_A4_HEIGHT, &info);

	struct slide *sl = malloc(sizeof(struct slide));
	int slide_count = 0;

	while (read_next_slide(fp, sl)) {
		add_slide(pdf, sl, page_width, page_height, dark_mode);
		slide_count++;
	}

	free(sl);
	fclose(fp);

	pdf_save(pdf, output_file_name);
	pdf_destroy(pdf);

	clock_t diff = clock() - start;
	int msec = diff * 1000 / CLOCKS_PER_SEC;
	printf("rendered %d slides in %dms\n", slide_count, msec);
	return 0;
}
