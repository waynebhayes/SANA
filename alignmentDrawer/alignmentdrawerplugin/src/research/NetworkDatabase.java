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

import java.util.HashMap;
import java.util.HashSet;
import java.util.Set;

/**
 * Manage all storing data for the networks
 *
 * @author davis
 */
public class NetworkDatabase {

        private final HashMap<Long, Set<Bindable>> m_bindings;

        NetworkDatabase() {
                m_bindings = new HashMap<>();
        }

        void add_network_bindings(AlignmentNetwork network, Bindable bindable) {
                Set<Bindable> s = m_bindings.get(network.get_suid());
                if (s != null) {
                        // has this network already
                        s.add(bindable);
                } else {
                        // first seen this network
                        HashSet<Bindable> new_set = new HashSet<>(1);
                        new_set.add(bindable);
                        m_bindings.put(network.get_suid(), new_set);
                }
        }

        Bindable get_network_binding(AlignmentNetwork network, String bindable_id) {
                Set<Bindable> bindables = m_bindings.get(network.get_suid());
                if (bindables != null) {
                        for (Bindable b : bindables) {
                                if (b.get_id().equals(bindable_id)) {
                                        return b;
                                }
                        }
                        return null;
                } else {
                        return null;
                }
        }

        boolean has_network(AlignmentNetwork network) {
                return m_bindings.containsKey(network.get_suid());
        }
}
