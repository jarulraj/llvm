#include <stdio.h>
#include <stdlib.h>

static unsigned long *get_crc_table( unsigned long *crc_table )
{
    // First call to this function? Or after freeing the memory?
    if ( !crc_table )
    {
        // Allocate memory
        crc_table = malloc(sizeof(unsigned long) * 256);

        // Was the allocation successful?
        if ( crc_table )
        {
            // Generate the crc table
            unsigned long crc ;
            int i, j ;

            for(i = 0; i < 256; i++)
            {
                crc = i;
                for (j = 8; j > 0; j--)
                {
                    if (crc & 1) crc = (crc >> 1) ^ 0xEDB88320UL;
                    else         crc >>= 1;
                }
                crc_table[i] = crc;
            }
        }
    }

    // Return the new pointer
    return crc_table ;
}

unsigned long get_crc32(const char *file)
{
    static unsigned long *crc_table = NULL;
    unsigned long crc = 0;

    // Called with '0'?
    if (!file)
    {
        // If we have a crc table, delete it from memory
        free(crc_table);

        // Set it to a null pointer, the have it (re)created on next calls to this
        // function
        crc_table = NULL;
    }
    else
    {
        // Get the crc table, on first call, generate, otherwise do nothing
        crc_table = get_crc_table( crc_table ) ;
        
        // Do we have a crc table?
        if ( crc_table )
        {
            // Open the file for reading
            FILE *fp = fopen(file, "r");

            // Was the file open successful?
            if (fp)
            {
                // Calculate the checksum
                int ch;
                
                crc = 0xFFFFFFFFUL;
                while ((ch = getc(fp)) != EOF)
                    { crc = (crc>>8) ^ crc_table[ (crc^ch) & 0xFF ]; }

                crc ^= 0xFFFFFFFFUL ;

                // Close the file
                fclose(fp);
            }
        }
    }

    // Return the checksum result
    return crc ;
}
