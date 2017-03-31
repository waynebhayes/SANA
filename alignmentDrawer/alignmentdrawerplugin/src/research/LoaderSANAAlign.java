/* 
 * Copyright (C) 2015 Wen, Chifeng <https://sourceforge.net/u/daviesx/profile/>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
package research;

import java.io.InputStream;
import java.util.HashSet;
import java.util.Set;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import org.cytoscape.io.DataCategory;
import org.cytoscape.io.read.CyTableReader;
import org.cytoscape.model.CyTable;
import org.cytoscape.model.CyTableFactory;
import org.cytoscape.work.TaskMonitor;

/**
 * Actual implementation of a SANA alignment file loader.
 *
 * @author Wen, Chifeng <https://sourceforge.net/u/daviesx/profile/>
 */
public class LoaderSANAAlign implements LoaderProtocol, CyTableReader {

        private final String c_AlignmentFileExtension = "sanalign";
        private final String c_AlignmentFileContent = "txt";
        private final DataCategory c_AlignmentFileCategory = DataCategory.TABLE;
        private final String c_AlignmentFileDesc = "SANA Network Alignment File";
        private final String c_AlignmentFileLoaderID = "SANAAlignmentLoader";

        private HashSet<String> m_extenstion = null;
        private HashSet<String> m_content = null;

        private CyTableFactory m_table_fact = null;
        private CyTable m_table = null;
        private NetworkAligner m_aligner = null;
        private String m_filename = "";
        private InputStream m_istream = null;
        private String m_data_name_override = "";

        private boolean m_is_canceled = false;

        public LoaderSANAAlign() {
                m_extenstion = new HashSet<>();
                m_content = new HashSet<>();
                m_extenstion.add(c_AlignmentFileExtension);
                m_content.add(c_AlignmentFileContent);
        }

        public void override_data_name(String name) {
                m_data_name_override = name;
        }

        @Override
        public void run(TaskMonitor tm) throws Exception {
                m_is_canceled = false;
                System.out.println(getClass() + " - Running sana alignment Loader...");
                tm.setTitle("Loading in sana alignment data...");

                if (m_aligner == null) {
                        throw new Exception("Object NetworkAligner is not constructed yet");
                }
                m_aligner.set_data_name(m_data_name_override + "<" + m_filename + ">");
                String[] lines = Util.extract_lines_from_stream(m_istream);

                // keep track of progress
                int j = 0;
                int total = lines.length;

                Pattern regex_pattern
                        = Pattern.compile("([[a-zA-Z][0-9][-][@][,][\"]]*) ([[a-zA-Z][0-9][-][@][,][\"]]*)");
                for (String line : lines) {
                        if (m_is_canceled) {
                                System.out.println(getClass() + " - Task is canceled.");
                                return;
                        }
                        Matcher matcher = regex_pattern.matcher(line);
                        if (!matcher.matches()) {
                                // The alignment file is invalid
                                throw new Exception("The alignement file contains illegal formatted line: " + line);
                        }
                        String sn0 = matcher.group(1);
                        String sn1 = matcher.group(2);
                        m_aligner.add_data_aligned_node_pair(sn0, sn1);
                        Util.advance_progress(tm, j, total);
                }

                m_table = m_aligner.get_data();
                System.out.println(getClass() + " - Finished reading sana alignment data...");
                System.out.println(getClass() + " - Everything has been loaded.");
        }

        @Override
        public void cancel() {
                m_is_canceled = true;
        }

        @Override
        public CyTable[] getTables() {
                return new CyTable[]{m_table};
        }

        @Override
        public Set<String> get_file_extension() {
                return m_extenstion;
        }

        @Override
        public Set<String> get_file_content_type() {
                return m_content;
        }

        @Override
        public DataCategory get_file_category() {
                return c_AlignmentFileCategory;
        }

        @Override
        public String get_file_description() {
                return c_AlignmentFileDesc;
        }

        @Override
        public String get_file_loader_id() {
                return c_AlignmentFileLoaderID;
        }

        @Override
        public void set_input_stream(InputStream s) {
                m_istream = s;
        }
        
        @Override
        public void set_file_name(String filename) {
                m_filename = filename;
        }

        @Override
        public void set_loader_service(CytoscapeLoaderService service) {
                m_table_fact = service.get_table_factory();
                m_aligner = new NetworkAligner(m_table_fact);
                m_table = m_aligner.get_data();
        }
}
