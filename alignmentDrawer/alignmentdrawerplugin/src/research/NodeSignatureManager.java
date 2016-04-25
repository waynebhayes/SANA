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
import java.util.List;
import java.util.Set;
import org.apache.commons.collections4.SetUtils;
import org.apache.commons.collections4.bidimap.DualLinkedHashBidiMap;

/**
 * Manage signature identification.
 *
 * @author davis
 */
public class NodeSignatureManager {

//        private final LinkedHashSet<String> m_namespaces = new LinkedHashSet<>();
//        private final LinkedHashSet<String> m_ids = new LinkedHashSet<>();
        private final DualLinkedHashBidiMap<String, String> m_node_id = new DualLinkedHashBidiMap<>();

        private final String COMMA = "#";
        private final String SEPARATOR = "/";

        private void arrange_id_namespace(String sig_string) {
                String[] parts = sig_string.split(SEPARATOR);
                if (parts.length != 2)
                        return;
                String[] namespaces = parts[0].split(COMMA);
                String[] ids = parts[1].split(COMMA);
                for (int i = 0; i < ids.length; i ++) {
                        if (namespaces[i].equals("") || ids[i].equals(""))
                                continue;
                        m_node_id.put(namespaces[i], ids[i]);
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
                clear();
                arrange_id_namespace(sig_string);
        }
        
        public void append_with(String sig_string) {
                arrange_id_namespace(sig_string);
        }
        
        public void clear() {
                m_node_id.clear();
        }
        
        public void add_namespaced_id(String namespace, String id) {
                m_node_id.put(namespace, id);
        }
        
        public Set<String> get_all_ids() {
                return m_node_id.values();
        }
        
        public Set<String> get_all_namespaces() {
                return m_node_id.keySet();
        }

        @Override
        public boolean equals(Object o) {
                if (!(o instanceof NodeSignatureManager)) {
                        return false;
                }
                NodeSignatureManager other = (NodeSignatureManager) o;
                Set<String> common_ids = SetUtils.intersection(get_all_ids(), other.get_all_ids());
                for (String common_id : common_ids) {
                        if (m_node_id.getKey(common_id).equals(other.m_node_id.getKey(common_id))) {
                                return true;
                        }
                }
                return false;
        }

        @Override
        public int hashCode() {
//                int hash = 7;
//                hash = 53 * hash + Objects.hashCode(this.m_namespaces);
//                hash = 53 * hash + Objects.hashCode(this.m_ids);
                return 0;
        }

        @Override
        public String toString() {
                StringBuilder s = new StringBuilder();
                List<String> keys = new ArrayList<>();
                for (String ns : m_node_id.keySet()) {
                        keys.add(ns);
                        s.append(ns).append(COMMA);
                }
                s.append(SEPARATOR);
                for (String namespace : keys) {
                        s.append(m_node_id.get(namespace)).append(COMMA);
                }
                return s.toString();
        }

        public String toSimplifiedString() {
                StringBuilder s = new StringBuilder();
                for (String id : m_node_id.values()) {
                        s.append(id).append(COMMA);
                }
                return s.toString();
        }
}
