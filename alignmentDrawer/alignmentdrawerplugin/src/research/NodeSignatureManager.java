/* 
 * Copyright (C) 2016 Wen, Chifeng <https://sourceforge.net/u/daviesx/profile/>
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

/**
 * Manage signature identification.
 *
 * @author davis
 */
public class NodeSignatureManager implements Comparable {
        
        private final String COMMA = "-";
        private final String SEPARATOR = ":";
        
       public class NodeSignature {
               protected final String namespace;
               protected final String id;
               private final int m_hash;

               public NodeSignature(String namespace, String id) {
                       this.namespace = namespace;
                       this.id = id;
                       m_hash = namespace.hashCode() + id.hashCode();
               }
               
               @Override
               public boolean equals(Object o) {
                       NodeSignature other = (NodeSignature) o;
                       return id.equals(other.id) && namespace.equals(other.namespace);
               }

                @Override
                public int hashCode() {
                        return m_hash;
                }
                
               @Override
                public String toString() {
                        return namespace + SEPARATOR + id;
                }
       }
       
       private final ArrayList<NodeSignature> m_sigs = new ArrayList<>();
        
       private void __add_signature(String namespace, String id) {
                for (NodeSignature sig : m_sigs) {
                        if (id.hashCode() + namespace.hashCode() == sig.hashCode() &&
                            sig.id.equals(id) && sig.namespace.equals(namespace)) {
                                return;
                        }
                }
                m_sigs.add(new NodeSignature(namespace, id));
        }
        
        private void __clean_signatures() {
                m_sigs.clear();
        }

        private void __arrange_id_namespace(String sig_string) {
                String[] parts = sig_string.split(COMMA);
                for (String part : parts) {
                        String[] nsid = part.split(SEPARATOR);
                        if (nsid.length == 2)
                                __add_signature(nsid[0], nsid[1]);
                }
        }

        public NodeSignatureManager() {
        }
        
//        public NodeSignatureManager(String output) {
//                try {
//                        arrange_id_namespace(output);
//                } catch (Exception ex) {
//                        System.out.println(getClass() + " - Error: " + output);
//                        // throw ex;
//                }
//        }
        
        public void override_with(String sig_string) {
                __clean_signatures();
                __arrange_id_namespace(sig_string);
        }
        
        public void append_with(String sig_string) {
                __arrange_id_namespace(sig_string);
        }
        
        public void clear() {
                __clean_signatures();
        }
        
        public void add_namespaced_id(String namespace, String id) {
                __add_signature(namespace, id);
        }
        
        public ArrayList<NodeSignature> get_all_node_signatures() {
                return m_sigs;
        }

        @Override
        public boolean equals(Object o) {
                NodeSignatureManager other = (NodeSignatureManager) o;
                for (NodeSignature sig0 : m_sigs) {
                        for (NodeSignature sig1 : other.m_sigs) {
                                if (sig0.hashCode() == sig1.hashCode() && sig0.equals(sig1))
                                        return true;
                        }
                }
                return false;
        }
        
        @Override
        public int compareTo(Object o) {
                return equals(o) ? 0 : 1;
        }

        @Override
        public int hashCode() {
                return 0;
        }

        @Override
        public String toString() {
                if (m_sigs.isEmpty()) return "";
                StringBuilder s = new StringBuilder();
                NodeSignature first = m_sigs.get(0);
                s.append(first.namespace).append(SEPARATOR).append(first.id);
                for (int i = 1; i < m_sigs.size(); i ++) {
                        NodeSignature sig = m_sigs.get(i);
                        s.append(COMMA).append(sig.namespace).append(SEPARATOR).append(sig.id);
                }
                return s.toString();
        }

        public String toSimplifiedString() {
                if (m_sigs.isEmpty()) return "";
                StringBuilder s = new StringBuilder();
                NodeSignature first = m_sigs.get(0);
                s.append(first.id);
                for (int i = 1; i < m_sigs.size(); i ++) {
                        NodeSignature sig = m_sigs.get(i);
                        s.append(COMMA).append(sig.id);
                }
                return s.toString();
        }
}
