CC      = gcc
CFLAGS  = -I.
RM      = rm -f
OBJ     = main.c priv.c util.c database.c
LIBS    = `/usr/bin/mysql_config --libs`
BINDIR  = /usr/local/bin

lbcd: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

default: all

all: lbcd 

.PHONY: install
install:
    sed 's/BINPATH/$(BINDIR)/g' $(PWD)/lbc > $(PWD)/lbc.1
    sed 's/BINPATH/$(BINDIR)/g' $(PWD)/lbc_service > $(PWD)/lbc_service.1
    install -m 0750 $(PWD)/lbc.1 $(BINDIR)/lbc
    install -m 0750 $(PWD)/lbcd $(BINDIR)       
    install -m 0700 $(PWD)/lbc_service.1 /etc/systemd/system/lbc
    systemctl daemon-reload
    systemctl enable lbc
    systemctl start lbc

.PHONY: clean
clean:
    rm -f $@
    rm -f *.1   
