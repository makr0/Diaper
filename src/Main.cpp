/*
	
	The caller function for Diaper
	Released under GPL v 2.0 AND on authors choice a later version, if it comes
*/


#include <iostream>
#include <assert.h>

#include "Diaper.h"


int main(
		 int argc,
		 char *argv[]
		)


{

	// This user code has to deal with the exceptions.

		Diaper dia;

		if ( argc != 3 )	{
			std::cout << "usage:diaper <input_dia_file> <output_sql_file>\n";
			return -1;
		}

		std::cout << "Diaper: A tool to convert dia based ER diagram to SQL statements\n"
					<< "Please read user manual before using\n";
		try {
			dia.parse_File(argv[1]);
		}
		catch(std::ios::failure fail)		{
			std::cerr << "ERROR::Could not open input file " << argv[1] << '\n';
			exit(-1);	
		}
		catch ( Diaper_Exception diaper_Ex)	{
			std::cerr << diaper_Ex.what() << '\n';
			exit(-1);
		}
		try {
			dia.write_Sql(argv[2]);
		}
		catch ( std::ios::failure fail )	{
			std::cerr << "ERROR:: Could not open Output File "<< argv[2] << '\n';
			exit(-1);
		}
		catch ( Diaper_Exception dia_ex ) {
			std::cerr << dia_ex.what() << '\n';
			exit(-1);
		}

		std::cout << "\nFor input file " << argv[1] << " output file " << argv[2] << '\n';
		return 0;
}
