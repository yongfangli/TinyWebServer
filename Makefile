obj = doit.o tiny.o

func = read_requesthdrs.o parse_uri.o serve_static.o serve_dynamic.o get_filetype.o clienterror.o

head = ../../mylib/csapp.h

tinyserver : $(obj)
	@gcc-7 -o tinyserver $(obj)

tiny.o : server.h $(head)

doit.o : $(func)

read_requesthdrs.o : $(head)

parse_uri.o : $(head)

serve_static.o : $(head)

serve_dynamic.o : $(head)

get_filetype.o : $(head)

clienterror.o : $(head)
