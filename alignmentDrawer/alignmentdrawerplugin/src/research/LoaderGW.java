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
import java.util.ArrayList;
import java.util.HashSet;
import java.util.Set;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import org.cytoscape.io.DataCategory;
import org.cytoscape.io.read.CyNetworkReader;
import org.cytoscape.model.CyEdge;
import org.cytoscape.model.CyNetwork;
import org.cytoscape.model.CyNetworkFactory;
import org.cytoscape.model.CyNode;
import org.cytoscape.view.model.CyNetworkView;
import org.cytoscape.view.model.CyNetworkViewFactory;
import org.cytoscape.work.TaskMonitor;

/**
 * Actually implementation of GW file loader
 *
 * @author Wen, Chifeng <https://sourceforge.net/u/daviesx/profile/>
 */
public class LoaderGW implements CyNetworkReader, LoaderProtocol {

        private final String c_GWFileExtension = "gw";
        private final String c_GWFileContent = "txt";
        private final DataCategory c_GWFileCategory = DataCategory.NETWORK;
        private final String c_GWFileDesc = "GW LEDA network file";
        private final String c_GWFileLoaderID = "GWLoader";

        private HashSet<String> m_extenstion = null;
        private HashSet<String> m_content = null;

        private CyNetworkViewFactory m_network_view_fact = null;
        private CyNetworkFactory m_network_fact = null;
        private String m_filename = "";
        private InputStream m_istream = null;
        private CyNetwork m_network = null;
        private boolean m_is_canceled = false;

        public LoaderGW() {
                m_extenstion = new HashSet<>();
                m_content = new HashSet<>();
                m_extenstion.add(c_GWFileExtension);
                m_content.add(c_GWFileContent);
        }

        @Override
        public CyNetwork[] getNetworks() {
                return new CyNetwork[]{m_network};
        }

        @Override
        public CyNetworkView buildCyNetworkView(CyNetwork cn) {
                if (m_network_view_fact == null) {
                        System.out.println("Have not given a network view factory");
                        return null;
                } else {
                        return m_network_view_fact.createNetworkView(cn);
                }
        }

        @Override
        public void run(TaskMonitor tm) throws Exception {
                System.out.println(getClass() + " - Running GW Loader...");
                tm.setTitle("Loading GW file...");

                m_is_canceled = false;

                String[] lines = Util.extract_lines_from_stream(m_istream);

                // GW file header:
                // LEDA.GRAPH
                // string
                // short
                // -2
                final int c_MinLines = 5;
                final int c_StartingLine = 4;
                if (lines.length < c_MinLines || !lines[0].startsWith("LEDA.GRAPH")) {
                        throw new Exception("Invalid LEDA.GRAPH");
                }

                if (m_network_fact == null) {
                        throw new Exception("Have not given a network factory");
                }
                String namespace = m_filename;
                AlignmentNetwork network_mgr = new AlignmentNetwork(m_network_fact, namespace);

                // keeep track of progress
                int j = 0;
                int total = lines.length;

                // load in nodes
                tm.setStatusMessage("Loading in nodes...");
                int num_nodes = Integer.decode(lines[c_StartingLine]);
                ArrayList<CyNode> node_list = new ArrayList<>();
                String current_line;
                Pattern regex_pattern = Pattern.compile("\\|\\{([[a-zA-Z][0-9][-][@][,][\"]]*)\\}\\|");
                NodeSignatureManager sig_mgr = new NodeSignatureManager();
                int i;
                for (i = c_StartingLine + 1; i < lines.length; i++) {
                        if (m_is_canceled) {
                                System.out.println(getClass() + " - Tasks canceled.");
                                return;
                        }
                        current_line = lines[i];
                        // pattern of a line containing a node: |{A1BG}|
                        Matcher matcher = regex_pattern.matcher(current_line);
                        if (!matcher.matches()) {
                                // finished reading node list
                                break;
                        }
                        String sig_string = matcher.group(1);
                        // create a node with signature
                        sig_mgr.clear();
                        sig_mgr.add_namespaced_id(namespace, sig_string);
                        CyNode node = network_mgr.make_node(sig_mgr);
                        network_mgr.add_node_belongings(node, null);
                        node_list.add(node);
                        Util.advance_progress(tm, j, total);
                }
                System.out.println(getClass() + " - Finished reading node list...");
                // load in edges
                tm.setStatusMessage("Loading in edges...");
                int num_edges = Integer.decode(lines[i++]);
     //           Pattern regex_pattern2 = Pattern.compile("([0-9]*) ([0-9]*) [0-9]* \\|\\{*\\}\\|");
     //           Pattern regex_pattern2 = Pattern.compile("([0-9]*) ([0-9]*) [0-9]* \\|\\{[0-9]*\\}\\|");
                Pattern regex_pattern2 = Pattern.compile("([0-9]*) ([0-9]*) [0-9]* \\|\\{.*\\}\\|");
                for (; i < lines.length; i++) {
                        if (m_is_canceled) {
                                System.out.println(getClass() + " - Tasks canceled.");
                                return;
                        }
                        current_line = lines[i];
                        Matcher matcher = regex_pattern2.matcher(current_line);
                        if (!matcher.matches()) {
                                // finished reading edge list
                                break;
                        }
                        // pattern of a line containing an edge: 649 757 0 |{}|
                        String sn0 = matcher.group(1);
                        String sn1 = matcher.group(2);
                        int n0 = Integer.decode(sn0);
                        int n1 = Integer.decode(sn1);
                        CyNode node0 = node_list.get(n0 - 1);
                        CyNode node1 = node_list.get(n1 - 1);
                        CyEdge edge = network_mgr.make_edge(node0, node1);
                        network_mgr.add_edge_belongings(edge, null);
                        Util.advance_progress(tm, j, total);
                }
                System.out.println(getClass() + " - Finished reading edge list...");

                m_network = network_mgr.get_network();

                if (m_network.getNodeCount() != num_nodes) {
                        throw new Exception("node count stated in file doesn't match that of loaded");
//                        System.out.println(getClass() 
//                                        + " - node count stated in file doesn't match that of loaded: "
//                                        + num_nodes + " != " + m_network.getNodeCount());
                }
                if (m_network.getEdgeCount() != num_edges) {
                        throw new Exception("edge count stated in file doesn't match that have been made");
//                        System.out.println(getClass() 
//                                        + " - edge count stated in file doesn't match that have been made: "
//                                        + num_edges + " != " + m_network.getEdgeCount());
                }
                
                NetworkDatabase db = NetworkDatabaseSingleton.get_instance();
                AlignmentNetwork align_net = new AlignmentNetwork(m_network);
                db.add_network_bindings(align_net, new Bindable(align_net, NetworkDatabase.BINDABLE_ID_NETWORK));
                db.update();
                
                System.out.println(getClass() + " - Everything has been loaded.");
        }

        @Override
        public void cancel() {
                m_is_canceled = true;
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
                return c_GWFileCategory;
        }

        @Override
        public String get_file_description() {
                return c_GWFileDesc;
        }

        @Override
        public String get_file_loader_id() {
                return c_GWFileLoaderID;
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
                m_network_fact = service.get_network_factory();
                m_network_view_fact = service.get_network_view_factory();
        }
}
