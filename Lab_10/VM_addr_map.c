#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define MAXSTR 1000

int main(int argc, char *argv[])
{
    char line[MAXSTR];
    int *page_table, *mem_map;
    unsigned int log_size, phy_size, page_size, d;
    unsigned int num_pages, num_frames;
    unsigned int offset, logical_addr, physical_addr, page_num, frame_num;

    /* Get the memory characteristics from the input file */
    fgets(line, MAXSTR, stdin);
    if ((sscanf(line, "Logical address space size: %d^%d", &d, &log_size)) != 2)
    {
        fprintf(stderr, "Unexpected line 1. Abort.\n");
        exit(-1);
    }
    fgets(line, MAXSTR, stdin);
    if ((sscanf(line, "Physical address space size: %d^%d", &d, &phy_size)) != 2)
    {
        fprintf(stderr, "Unexpected line 2. Abort.\n");
        exit(-1);
    }
    fgets(line, MAXSTR, stdin);
    if ((sscanf(line, "Page size: %d^%d", &d, &page_size)) != 2)
    {
        fprintf(stderr, "Unexpected line 3. Abort.\n");
        exit(-1);
    }

    /* Calculate the number of pages and frames */
    num_pages = 1 << (log_size - page_size);
    num_frames = 1 << (phy_size - page_size);
    
    /* Allocate arrays to hold the page table and memory frames map */
    page_table = (int *)malloc(num_pages * sizeof(int));
    mem_map = (int *)malloc(num_frames * sizeof(int));

    /* Initialize page table to indicate that no pages are currently mapped to physical memory */
    for (int i = 0; i < num_pages; i++)
    {
        page_table[i] = -1;
    }

    /* Initialize memory map table to indicate no valid frames */
    for (int i = 0; i < num_frames; i++)
    {
        mem_map[i] = 0;
    }

    printf("Number of Pages: %d, Number of Frames: %d\n", num_pages, num_frames);

    /* Read each accessed address from input file. Map the logical address to corresponding physical address */
    fgets(line, MAXSTR, stdin);
    while (!(feof(stdin)))
    {
        sscanf(line, "0x%x", &logical_addr);
        fprintf(stdout, "\nLogical address: 0x%x\n", logical_addr);

        /* Calculate page number and offset from the logical address */
        page_num = logical_addr >> page_size;  //right shift by page size
        offset = logical_addr & ((1 << page_size) - 1); // masking

        printf("Page Number: %d\n", page_num);

        /* Check if the page is already in physical memory */
        if (page_table[page_num] == -1)
        {
            printf("Page Fault!\n");

            /* If not, find an available frame and map the page to that frame */
            for (int i = 0; i < num_frames; i++)
            {
                if (mem_map[i] == 0)
                {
                    page_table[page_num] = i;
                    mem_map[i] = 1;
                    printf("Frame number: %d\n", i);
                    break;
                }
            }
        }
        else
        {
            printf("Frame number: %d\n", page_table[page_num]);
        }

        /* Form corresponding physical address */
        frame_num = page_table[page_num];
        physical_addr = (frame_num << page_size) | offset; // leftshift frame number to page size 

        fprintf(stdout, "Physical address: 0x%x\n", physical_addr);

        /* Read next line */
        fgets(line, MAXSTR, stdin);
    }

    /* Free allocated memory */
    free(page_table);
    free(mem_map);

    return 0;
}
