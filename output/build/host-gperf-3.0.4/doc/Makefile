# Makefile for gperf/doc

# Copyright (C) 1998, 2000, 2002-2003, 2007 Free Software Foundation, Inc.
# Written by Douglas C. Schmidt <schmidt@ics.uci.edu>
# and Bruno Haible <bruno@clisp.org>.
#
# This file is part of GNU GPERF.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

#### Start of system configuration section. ####

# Directories used by "make":
srcdir = .

# Directories used by "make install":
prefix = /home/simon/GIT/buildroot-k3-current/output/host/usr
local_prefix = /usr/local
exec_prefix = ${prefix}
datarootdir = ${prefix}/share
datadir = ${datarootdir}
infodir = ${datarootdir}/info
mandir = ${datarootdir}/man
man1dir = $(mandir)/man1
docdir = ${datarootdir}/doc/${PACKAGE}
dvidir = ${docdir}
psdir = ${docdir}
pdfdir = ${docdir}
htmldir = ${docdir}

# Programs used by "make":
RM = rm -f


# Programs used by "make" if you have changed the documentation files:
TEX = tex
TEXI2DVI = texi2dvi
DVIPS = dvips -D600
TEXI2PDF = texi2pdf
MAKEINFO = LANG= LC_MESSAGES= LC_ALL= LANGUAGE= makeinfo
TEXI2HTML = perl $(srcdir)/texi2html

# Programs used by "make install":
INSTALL = /usr/bin/install -c
INSTALL_PROGRAM = $(INSTALL)
INSTALL_DATA = $(INSTALL) -m 644
MKINSTALLDIRS = $(SHELL) $(srcdir)/../mkinstalldirs

#### End of system configuration section. ####

SHELL = /bin/sh



all : info ps pdf html


info : $(srcdir)/gperf.info

$(srcdir)/gperf.info : $(srcdir)/gperf.texi $(srcdir)/gpl-3.0.texi
	cd $(srcdir) && $(MAKEINFO) --no-split gperf.texi


dvi : $(srcdir)/gperf.dvi

$(srcdir)/gperf.dvi : $(srcdir)/gperf.texi $(srcdir)/gpl-3.0.texi
	cd $(srcdir) && $(RM) gperf.aux gperf.toc gperf.cp gperf.fn gperf.ky gperf.pg gperf.tp gperf.vr gperf.log gperf.cps
	cd $(srcdir) && $(TEXI2DVI) gperf.texi
	cd $(srcdir) && $(RM) gperf.aux gperf.toc gperf.cp gperf.fn gperf.ky gperf.pg gperf.tp gperf.vr gperf.log gperf.cps


ps : $(srcdir)/gperf.ps

$(srcdir)/gperf.ps : $(srcdir)/gperf.dvi
	$(DVIPS) -o $@ $(srcdir)/gperf.dvi


pdf : $(srcdir)/gperf.pdf

$(srcdir)/gperf.pdf : $(srcdir)/gperf.texi $(srcdir)/gpl-3.0.texi
	cd $(srcdir) && $(RM) gperf.aux gperf.toc gperf.cp gperf.fn gperf.ky gperf.pg gperf.tp gperf.vr gperf.log gperf.cps
	cd $(srcdir) && $(TEXI2PDF) gperf.texi
	cd $(srcdir) && $(RM) gperf.aux gperf.toc gperf.cp gperf.fn gperf.ky gperf.pg gperf.tp gperf.vr gperf.log gperf.cps


html : gperf.html gperf_toc.html

gperf.html : $(srcdir)/gperf.texi $(srcdir)/gpl-3.0.texi
	cd $(srcdir) && $(TEXI2HTML) -number -monolithic gperf.texi

gperf_toc.html : $(srcdir)/gperf.texi $(srcdir)/gpl-3.0.texi
	cd $(srcdir) && $(RM) gperf_*.html
	cd $(srcdir) && $(TEXI2HTML) -number -split_chapter gperf.texi


install : all force
	$(MKINSTALLDIRS) $(DESTDIR)$(infodir)
	$(INSTALL_DATA) $(srcdir)/gperf.info $(DESTDIR)$(infodir)/gperf.info
	$(MKINSTALLDIRS) $(DESTDIR)$(man1dir)
	$(INSTALL_DATA) $(srcdir)/gperf.1 $(DESTDIR)$(man1dir)/gperf.1
#	$(MKINSTALLDIRS) $(DESTDIR)$(dvidir)
#	$(INSTALL_DATA) $(srcdir)/gperf.dvi $(DESTDIR)$(dvidir)/gperf.dvi
#	$(MKINSTALLDIRS) $(DESTDIR)$(psdir)
#	$(INSTALL_DATA) $(srcdir)/gperf.ps $(DESTDIR)$(psdir)/gperf.ps
#	$(MKINSTALLDIRS) $(DESTDIR)$(pdfdir)
#	$(INSTALL_DATA) $(srcdir)/gperf.pdf $(DESTDIR)$(pdfdir)/gperf.pdf
	$(MKINSTALLDIRS) $(DESTDIR)$(htmldir)
	$(INSTALL_DATA) $(srcdir)/gperf.html $(DESTDIR)$(htmldir)/gperf.html

installdirs : force
	$(MKINSTALLDIRS) $(DESTDIR)$(infodir)
	$(MKINSTALLDIRS) $(DESTDIR)$(man1dir)
#	$(MKINSTALLDIRS) $(DESTDIR)$(dvidir)
#	$(MKINSTALLDIRS) $(DESTDIR)$(psdir)
#	$(MKINSTALLDIRS) $(DESTDIR)$(pdfdir)
	$(MKINSTALLDIRS) $(DESTDIR)$(htmldir)

uninstall : force
	$(RM) $(DESTDIR)$(infodir)/gperf.info
	$(RM) $(DESTDIR)$(man1dir)/gperf.1
#	$(RM) $(DESTDIR)$(dvidir)/gperf.dvi
#	$(RM) $(DESTDIR)$(psdir)/gperf.ps
#	$(RM) $(DESTDIR)$(pdfdir)/gperf.pdf
	$(RM) $(DESTDIR)$(htmldir)/gperf.html

check : all

mostlyclean : clean

clean : force
	$(RM) *~ *.aux *.toc *.cp *.fn *.ky *.pg *.tp *.vr *.my *.log *.cps core

distclean : clean
	$(RM) config.status config.log config.cache Makefile

maintainer-clean : distclean
	$(RM) *.info *.dvi *.ps *.pdf *.html

force :

