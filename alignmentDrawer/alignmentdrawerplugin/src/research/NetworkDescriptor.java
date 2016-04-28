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

import java.util.ArrayList;
import java.util.HashSet;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Set;
import org.cytoscape.model.CyEdge;
import org.cytoscape.model.CyNode;

/**
 * Put customize styling for the aligned network.
 *
 * @author Wen, Chifeng <https://sourceforge.net/u/daviesx/profile/>
 */
public class NetworkDescriptor {

        private final String m_style_name;
        private final AlignmentNetwork m_network;

        private final LinkedHashSet<CyNode> m_nodes = new LinkedHashSet<>();
        private final LinkedHashSet<CyEdge> m_edges = new LinkedHashSet<>();
        private final ArrayList<NodeSignatureManager> m_node_sigs = new ArrayList<>();

        public NetworkDescriptor(AlignmentNetwork network) {
                m_network = network;
                m_style_name = m_network.get_suggested_name() + "_visual_style";
        }

        public void select_all() {
                // select nodes
                for (AlignmentNetwork.NodeIterator i = m_network.NodeIterator(); i.hasNext();) {
                        String sig = i.next();
                        NodeSignatureManager sig_mgr = new NodeSignatureManager();
                        sig_mgr.override_with(sig);
                        CyNode node = m_network.get_node_from_signature(sig_mgr);
                        m_nodes.add(node);
                        m_node_sigs.add(sig_mgr);
                }
                // select edges
                for (AlignmentNetwork.EdgeIterator i = m_network.EdgeIterator(); i.hasNext();) {
                        AlignmentNetwork.Edge euid = i.next();
                        CyEdge edge = m_network.get_edge_from_suid(euid.m_eid);
                        m_edges.add(edge);
                }
        }

        public void select_by_belongings(Set<AlignmentNetwork> belnets,
                                         boolean is_complement,
                                         boolean with_neighbour_edges) {
                HashSet<Long> belids = new HashSet<>();
                for (AlignmentNetwork net : belnets) {
                        belids.add(net.get_suid());
                }
                // select nodes
                for (AlignmentNetwork.NodeIterator i = m_network.NodeIterator(); i.hasNext();) {
                        String sig = i.next();
                        NodeSignatureManager sig_mgr = new NodeSignatureManager();
                        sig_mgr.override_with(sig);
                        CyNode node = m_network.get_node_from_signature(sig_mgr);

                        Set<Long> belongings = Util.list_to_set(m_network.get_node_belongings(node));
                        boolean cond = (!is_complement && belids.equals(belongings))
                                       || (is_complement && !belids.equals(belongings));
                        if (cond) {
                                m_nodes.add(node);
                                m_node_sigs.add(sig_mgr);
                        }
                }
                // select edges
                NodeSignatureManager sig_mgr = new NodeSignatureManager();
                for (AlignmentNetwork.EdgeIterator i = m_network.EdgeIterator(); i.hasNext();) {
                        AlignmentNetwork.Edge euid = i.next();
                        CyEdge edge = m_network.get_edge_from_suid(euid.m_eid);
                        Set<Long> belongings = Util.list_to_set(m_network.get_edge_belongings(edge));
                        boolean cond = (!is_complement && belids.equals(belongings))
                                       || (is_complement && !belids.equals(belongings));
                        if (cond) {
                                m_edges.add(edge);
                                if (with_neighbour_edges) {
                                        // neighbours as well
                                        sig_mgr.override_with(euid.m_e0);
                                        CyNode n0 = m_network.get_node_from_signature(sig_mgr);
                                        sig_mgr.override_with(euid.m_e1);
                                        CyNode n1 = m_network.get_node_from_signature(sig_mgr);

                                        List<CyEdge> n0e = m_network.get_neighbour_edges(n0);
                                        List<CyEdge> n1e = m_network.get_neighbour_edges(n1);

                                        m_edges.addAll(n0e);
                                        m_edges.addAll(n1e);
                                }
                        }
                }
        }

        public void select_by_signatures(Set<NodeSignatureManager> sigs,
                                         boolean is_complement,
                                         boolean with_neighbour_edges) {
                // select nodes
                for (AlignmentNetwork.NodeIterator i = m_network.NodeIterator(); i.hasNext();) {
                        String sig = i.next();
                        NodeSignatureManager sig_mgr = new NodeSignatureManager();
                        sig_mgr.override_with(sig);
                        CyNode node = m_network.get_node_from_signature(sig_mgr);

                        if (!is_complement) {
                                if (sigs.contains(sig_mgr)) {
                                        System.out.println(getClass() + " - Selected node: " + sig_mgr);
                                        m_nodes.add(node);
                                        m_node_sigs.add(sig_mgr);
                                }
                        } else if (!sigs.contains(sig_mgr)) {
                                m_nodes.add(node);
                                m_node_sigs.add(sig_mgr);
                        }
                }
                // select edges
                NodeSignatureManager e0 = new NodeSignatureManager();
                NodeSignatureManager e1 = new NodeSignatureManager();

                for (AlignmentNetwork.EdgeIterator i = m_network.EdgeIterator(); i.hasNext();) {
                        AlignmentNetwork.Edge euid = i.next();
                        CyEdge edge = m_network.get_edge_from_suid(euid.m_eid);

                        e0.override_with(euid.m_e0);
                        e1.override_with(euid.m_e1);

                        if (with_neighbour_edges) {
                                if (!is_complement) {
                                        if (sigs.contains(e0) || sigs.contains(e1)) {
                                                m_edges.add(edge);
                                        }
                                } else {
                                        if (!(sigs.contains(e0) || sigs.contains(e1))) {
                                                m_edges.add(edge);
                                        }
                                }
                        } else {
                                if (!is_complement) {
                                        if (sigs.contains(e0) && sigs.contains(e1)) {
                                                m_edges.add(edge);
                                        }
                                } else {
                                        if (!(sigs.contains(e0) && sigs.contains(e1))) {
                                                m_edges.add(edge);
                                        }
                                }
                        }
                }
        }

        public void select_by_belongings(Set<AlignmentNetwork> networks) {
                select_by_belongings(networks, false, false);
        }

        public void select_by_signatures(Set<NodeSignatureManager> sigs) {
                select_by_signatures(sigs, false, false);
        }

        public void select_by_complement_signatures(Set<NodeSignatureManager> sigs,
                                                    boolean without_neighbour) {
                select_by_signatures(sigs, without_neighbour, true);
        }

        public List<NodeSignatureManager> get_network_node_signatures() {
                return m_node_sigs;
        }

        public Set<CyNode> get_network_nodes() {
                return m_nodes;
        }

        public Set<CyEdge> get_network_edges() {
                return m_edges;
        }

        public void clear() {
                m_nodes.clear();
                m_edges.clear();
                m_node_sigs.clear();
        }

        public AlignmentNetwork get_alignment_network() {
                return m_network;
        }
}
