##README

Files:
alloc.c //Register Allocator, does all the work.

Generating an executable:
	Open terminal in directory
	enter 'make alloc'.
Running the file:
	Open terminal in directory
	enter ./alloc [k][a][f]
		[k]=number of registers, must be greater than 1.
		[a]=which allocator to use, 
			's' for simple topdown
			't' for topdown 
			'b' for bottom-up 
			'o' for optional (currently outputs the same as 's')
		[f]=filename of source iloc

	output is printed to stdout.

Cleaning up:
	Open terminal in directory
	enter 'make clean' 
		(Be careful, this will delete all .o files, or files named 'alloc')
