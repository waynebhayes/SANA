EXTRA_DIST += \
	doc/Doxyfile.in \
	doc/DoxygenLayout.xml \
	doc/coding_style.dox \
	doc/dirs.dox \
	doc/groups.dox \
	doc/lgf.dox \
	doc/license.dox \
	doc/mainpage.dox \
	doc/migration.dox \
	doc/min_cost_flow.dox \
	doc/named-param.dox \
	doc/namespaces.dox \
	doc/html \
	doc/CMakeLists.txt

DOC_EPS_IMAGES18 = \
	grid_graph.eps \
	nodeshape_0.eps \
	nodeshape_1.eps \
	nodeshape_2.eps \
	nodeshape_3.eps \
	nodeshape_4.eps

DOC_EPS_IMAGES27 = \
	bipartite_matching.eps \
	bipartite_partitions.eps \
	connected_components.eps \
	edge_biconnected_components.eps \
	matching.eps \
	node_biconnected_components.eps \
	planar.eps \
	strongly_connected_components.eps

DOC_EPS_IMAGES = \
	$(DOC_EPS_IMAGES18) \
	$(DOC_EPS_IMAGES27)

DOC_PNG_IMAGES = \
	$(DOC_EPS_IMAGES:%.eps=doc/gen-images/%.png)

EXTRA_DIST += $(DOC_EPS_IMAGES:%=doc/images/%)

doc/html:
	$(MAKE) $(AM_MAKEFLAGS) html

GS_COMMAND=gs -dNOPAUSE -dBATCH -q -dEPSCrop -dTextAlphaBits=4 -dGraphicsAlphaBits=4

$(DOC_EPS_IMAGES18:%.eps=doc/gen-images/%.png): doc/gen-images/%.png: doc/images/%.eps
	-mkdir doc/gen-images
	if test ${gs_found} = yes; then \
	  $(GS_COMMAND) -sDEVICE=pngalpha -r18 -sOutputFile=$@ $<; \
	else \
	  echo; \
	  echo "Ghostscript not found."; \
	  echo; \
	  exit 1; \
	fi

$(DOC_EPS_IMAGES27:%.eps=doc/gen-images/%.png): doc/gen-images/%.png: doc/images/%.eps
	-mkdir doc/gen-images
	if test ${gs_found} = yes; then \
	  $(GS_COMMAND) -sDEVICE=pngalpha -r27 -sOutputFile=$@ $<; \
	else \
	  echo; \
	  echo "Ghostscript not found."; \
	  echo; \
	  exit 1; \
	fi

references.dox: doc/references.bib
	if test ${python_found} = yes; then \
	  cd doc; \
	  python @abs_top_srcdir@/scripts/bib2dox.py @abs_top_builddir@/$< >$@; \
	  cd ..; \
	else \
	  echo; \
	  echo "Python not found."; \
	  echo; \
	  exit 1; \
	fi

html-local: $(DOC_PNG_IMAGES) references.dox
	if test ${doxygen_found} = yes; then \
	  cd doc; \
	  doxygen Doxyfile; \
	  cd ..; \
	else \
	  echo; \
	  echo "Doxygen not found."; \
	  echo; \
	  exit 1; \
	fi

clean-local:
	-rm -rf doc/html
	-rm -f doc/doxygen.log
	-rm -f $(DOC_PNG_IMAGES)
	-rm -rf doc/gen-images

update-external-tags:
	wget -O doc/libstdc++.tag.tmp http://gcc.gnu.org/onlinedocs/libstdc++/latest-doxygen/libstdc++.tag && \
	mv doc/libstdc++.tag.tmp doc/libstdc++.tag || \
	rm doc/libstdc++.tag.tmp

install-html-local: doc/html
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(DESTDIR)$(htmldir)/html
	for p in doc/html/*.{html,css,png,map,gif,tag} ; do \
	  f="`echo $$p | sed -e 's|^.*/||'`"; \
	  echo " $(INSTALL_DATA) $$p $(DESTDIR)$(htmldir)/html/$$f"; \
	  $(INSTALL_DATA) $$p $(DESTDIR)$(htmldir)/html/$$f; \
	done

uninstall-local:
	@$(NORMAL_UNINSTALL)
	for p in doc/html/*.{html,css,png,map,gif,tag} ; do \
	  f="`echo $$p | sed -e 's|^.*/||'`"; \
	  echo " rm -f $(DESTDIR)$(htmldir)/html/$$f"; \
	  rm -f $(DESTDIR)$(htmldir)/html/$$f; \
	done

.PHONY: update-external-tags
