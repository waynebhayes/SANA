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
import java.util.Collection;
import java.util.Iterator;
import java.util.List;
import java.util.Objects;
import org.cytoscape.model.CyEdge;
import org.cytoscape.model.CyIdentifiable;
import org.cytoscape.model.CyNetwork;
import org.cytoscape.model.CyNetworkFactory;
import org.cytoscape.model.CyNode;
import org.cytoscape.model.CyRow;
import org.cytoscape.model.CyTable;

/**
 * Definition of an Alignment Network
 *
 * @author Wen, Chifeng <https://sourceforge.net/u/daviesx/profile/>
 */
public class AlignmentNetwork {

        private final String c_NodeSignatureSlot = "AlignNodeSig";
        private final String c_BelongingSlot = "AlignBelonging";
        private final String c_SUIDReferenceSlot = ".SUID";
        private final String c_EdgeSignature0Slot = "AlignEdgeSig0";
        private final String c_EdgeSignature1Slot = "AlignEdgeSig1";
        private final String c_NetworkNamespaceSlot = "NetworkNamespace";
        private CyNetwork m_network = null;

        private boolean build_node_attributes(CyNetwork network) {
                boolean is_compatible = true;

                m_network = network;
                // Configure nodes
                CyTable node_table = m_network.getDefaultNodeTable();
                if (node_table.getColumn(c_NodeSignatureSlot) == null) {
                        is_compatible = false;
                        node_table.createColumn(c_NodeSignatureSlot, String.class, false);
                }
                if (node_table.getColumn(c_BelongingSlot) == null) {
                        is_compatible = false;
                        node_table.createListColumn(c_BelongingSlot, Long.class, false);
                }
                if (node_table.getColumn(c_SUIDReferenceSlot) == null) {
                        is_compatible = false;
                        node_table.createColumn(c_SUIDReferenceSlot, Long.class, false);
                }
                // Configure edges
                CyTable edge_table = m_network.getDefaultEdgeTable();
                if (edge_table.getColumn(c_BelongingSlot) == null) {
                        is_compatible = false;
                        edge_table.createListColumn(c_BelongingSlot, Long.class, false);
                }
                if (edge_table.getColumn(c_SUIDReferenceSlot) == null) {
                        is_compatible = false;
                        edge_table.createColumn(c_SUIDReferenceSlot, Long.class, false);
                }
                if (edge_table.getColumn(c_EdgeSignature0Slot) == null) {
                        is_compatible = false;
                        edge_table.createColumn(c_EdgeSignature0Slot, String.class, false);
                }
                if (edge_table.getColumn(c_EdgeSignature1Slot) == null) {
                        is_compatible = false;
                        edge_table.createColumn(c_EdgeSignature1Slot, String.class, false);
                }
                // Configure the table
                CyTable network_table = m_network.getDefaultNetworkTable();
                if (network_table.getColumn(c_NetworkNamespaceSlot) == null) {
                        is_compatible = false;
                        network_table.createColumn(c_NetworkNamespaceSlot, String.class, false);
                }
                return is_compatible;
        }

        /**
         * Copy constructor.
         * 
         * @param network the network to be copied.
         */
        public AlignmentNetwork(AlignmentNetwork network) {
                if (!build_node_attributes(network.get_network())) {
                        System.out.println(this.getClass() + " - This network is not a compatible alignment network");
                }
        }

        /**
         * This will import the CyNetwork and absorb all the network data.
         *
         * @param network network to be imported.
         */
        public AlignmentNetwork(CyNetwork network) {
                if (!build_node_attributes(network)) {
                        System.out.println(this.getClass() + " - This network is not a compatible alignment network");
                }
        }

        /**
         * This will create a new CyNetwork with building using attributes for
         * the data.
         *
         * @param fact network factory used to create new network.
         * @param namespace the name space of this network.
         */
        public AlignmentNetwork(CyNetworkFactory fact, String namespace) {
                build_node_attributes(fact.createNetwork());
                set_network_nemespace(namespace);
        }

        CyNetwork get_network() {
                return m_network;
        }

        Long get_suid() {
                return m_network.getSUID();
        }

        public CyNode make_node(NodeSignatureManager sig) {
                CyNode node = m_network.addNode();
                CyRow attri = m_network.getRow(node);
                attri.set(c_NodeSignatureSlot, sig.toString());
                attri.set(c_SUIDReferenceSlot, node.getSUID());
                return node;
        }

        private int add_object_belongings(CyIdentifiable object, CyNetwork network) {
                if (network == null) {
                        network = m_network;
                }
                CyRow attri = m_network.getRow(object);
                List<Long> belonging_list = attri.getList(c_BelongingSlot, Long.class);

                if (belonging_list == null) {
                        belonging_list = new ArrayList<>();
                }
                if (!belonging_list.contains(network.getSUID())) {
                        // add belonging if node hasn't yet belong to the network
                        belonging_list.add(network.getSUID());
                        attri.set(c_BelongingSlot, belonging_list);
                        return 1;
                } else {
                        return 0;
                }
        }

        private boolean is_object_belong_to(CyIdentifiable object, CyNetwork network) {
                if (network == null) {
                        network = m_network;
                }
                CyRow attri = m_network.getRow(object);
                List<Long> belonging_list = attri.getList(c_BelongingSlot, Long.class);

                return belonging_list != null && belonging_list.contains(network.getSUID());
        }

        public int add_node_belongings(CyNode node, CyNetwork network) {
                return add_object_belongings(node, network);
        }

        public boolean is_node_belong_to(CyNode node, CyNetwork network) {
                return is_object_belong_to(node, network);
        }

        private List<Long> get_object_belongings(CyIdentifiable object) {
                CyRow attri = m_network.getRow(object);
                List<Long> belonging_list = attri.getList(c_BelongingSlot, Long.class);
                return belonging_list;
        }

        public List<Long> get_node_belongings(CyNode node) {
                return get_object_belongings(node);
        }

        public List<Long> get_edge_belongings(CyEdge edge) {
                return get_object_belongings(edge);
        }

        public void set_node_selected(CyNode node, Boolean is_selected) {
                CyRow attri = m_network.getRow(node);
                attri.set(CyNetwork.SELECTED, is_selected);
        }

        public void mutate_node_signature(CyNode node, NodeSignatureManager new_sig) {
                CyRow attri = m_network.getRow(node);
                attri.set(c_NodeSignatureSlot, new_sig.toString());
        }

        public CyNode get_node_from_signature(NodeSignatureManager sig) {
                CyTable table = m_network.getDefaultNodeTable();
                Collection<CyRow> rows = table.getMatchingRows(c_NodeSignatureSlot, sig.toString());
//                List<CyRow> rows = table.getAllRows();
//                NodeSignatureManager sigtmp = new NodeSignatureManager();
//                for (CyRow row : rows) {
//                        String curr_sig = row.get(c_NodeSignatureSlot, String.class);
//                        sigtmp.import_signature(curr_sig, true);
//                        if (sigtmp.equals(sig)) {
//                                Long ref = row.get(c_SUIDReferenceSlot, Long.class);
//                                return m_network.getNode(ref);
//                        }
//                }
                if (rows.isEmpty()) {
                        return null;
                }
                CyRow row = rows.iterator().next();
                Long ref = row.get(c_SUIDReferenceSlot, Long.class);
                return m_network.getNode(ref);
        }

        public CyNode get_node_from_suid(Long suid) {
                return m_network.getNode(suid);
        }

        public CyEdge get_edge_from_suid(Long suid) {
                return m_network.getEdge(suid);
        }

        public CyEdge make_edge(CyNode node0, CyNode node1) {
                CyEdge edge = m_network.addEdge(node0, node1, false);
                CyRow edge_attri = m_network.getRow(edge);
                edge_attri.set(c_SUIDReferenceSlot, edge.getSUID());
                String sig0 = m_network.getRow(node0).get(c_NodeSignatureSlot, String.class);
                String sig1 = m_network.getRow(node1).get(c_NodeSignatureSlot, String.class);
                edge_attri.set(c_EdgeSignature0Slot, sig0);
                edge_attri.set(c_EdgeSignature1Slot, sig1);
                return edge;
        }

        public int add_edge_belongings(CyEdge edge, CyNetwork network) {
                return add_object_belongings(edge, network);
        }

        public String get_suggested_name() {
                return m_network.getRow(m_network).get(CyNetwork.NAME, String.class);
        }

        public void set_suggested_name(String name) {
                m_network.getRow(m_network).set(CyNetwork.NAME, name);
        }
        
        public String get_network_namespace() {
                return m_network.getRow(m_network).get(c_NetworkNamespaceSlot, String.class);
        }

        private void set_network_nemespace(String namespace) {
                m_network.getRow(m_network).set(c_NetworkNamespaceSlot, namespace);
        }

        public NodeIterator NodeIterator() {
                return new NodeIterator(m_network);
        }

        public EdgeIterator EdgeIterator() {
                return new EdgeIterator(m_network);
        }

        int get_node_count() {
                return m_network.getNodeCount();
        }

        int get_edge_count() {
                return m_network.getEdgeCount();
        }
        
        public List<CyEdge> get_neighbour_edges(CyNode node) {
                return m_network.getAdjacentEdgeList(node, CyEdge.Type.ANY);
        }
        
        public List<CyNode> get_neighbour_nodes(CyNode node) {
                return m_network.getNeighborList(node, CyEdge.Type.ANY);
        }
        
        @Override
        public boolean equals(Object o) {
                if (!(o instanceof AlignmentNetwork)) return false;
                AlignmentNetwork other = (AlignmentNetwork) o;
                return get_suid().equals(other.get_suid());
        }

        @Override
        public int hashCode() {
                int hash = 7;
                hash = 97 * hash + get_suid().intValue();
                return hash;
        }

        public class NodeIterator implements Iterator<String> {

                private final List<CyRow> m_rows;
                private final Iterator<CyRow> m_row_it;
                private final CyNetwork m_network;

                NodeIterator(CyNetwork network) {
                        m_network = network;
                        m_rows = m_network.getDefaultNodeTable().getAllRows();
                        m_row_it = m_rows.iterator();
                }

                @Override
                public boolean hasNext() {
                        return m_row_it.hasNext();
                }

                @Override
                public String next() {
                        return m_row_it.next().get(c_NodeSignatureSlot, String.class);
                }

                public Long next2() {
                        return m_row_it.next().get(c_SUIDReferenceSlot, Long.class);
                }

                @Override
                public void remove() {
                        m_row_it.remove();
                }
        }

        public class Edge {

                public String m_e0;
                public String m_e1;

                public Long m_eid;

                Edge(String e0, String e1, Long id) {
                        m_e0 = e0;
                        m_e1 = e1;
                        m_eid = id;
                }
        }

        public class EdgeIterator implements Iterator<Edge> {

                private final List<CyRow> m_rows;
                private final Iterator<CyRow> m_row_it;
                private final CyNetwork m_network;

                EdgeIterator(CyNetwork network) {
                        m_network = network;
                        m_rows = m_network.getDefaultEdgeTable().getAllRows();
                        m_row_it = m_rows.iterator();
                }

                @Override
                public boolean hasNext() {
                        return m_row_it.hasNext();
                }

                @Override
                public Edge next() {
                        CyRow edge_attri = m_row_it.next();
                        String sig0 = edge_attri.get(c_EdgeSignature0Slot, String.class);
                        String sig1 = edge_attri.get(c_EdgeSignature1Slot, String.class);
                        Long eid = edge_attri.get(c_SUIDReferenceSlot, Long.class);
                        return new Edge(sig0, sig1, eid);
                }

                @Override
                public void remove() {
                        m_row_it.remove();
                }
        }
}
