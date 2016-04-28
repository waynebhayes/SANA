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
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Set;
import org.cytoscape.app.swing.CySwingAppAdapter;
import org.cytoscape.model.CyNetwork;
import org.cytoscape.model.CyNetworkManager;
import org.cytoscape.model.CyRow;
import org.cytoscape.model.CyTable;
import org.cytoscape.model.CyTableFactory;
import org.cytoscape.model.CyTableManager;
import org.cytoscape.view.model.CyNetworkView;
import org.cytoscape.view.model.CyNetworkViewManager;

/**
 * Manage all persistent data for the networks
 *
 * @author Wen, Chifeng
 */
public class NetworkDatabase {

        private final HashMap<Long, Set<Bindable>> m_bindings;
        private final CyNetworkManager m_network_mgr;
        private final CyNetworkViewManager m_netview_mgr;
        
        private static final String NETWORKDB_TABLE = "__NetworkDBTable";
        private final String c_PrimaryKey = "NetworkUUID";
        private final String c_AttriNetworkName = "NetworkName";
        private final String c_AttriNetworkG0 = "NetworkG0UUID";
        private final String c_AttriNetworkG1 = "NetworkG1UUID";
        private final String c_AttriNetworkView = "NetworkViewUUID";
        private CyTable m_dbtable;
        
        public static final String BINDABLE_ID_NETWORK = "__NetworkBindable";
        public static final String BINDABLE_ID_NETWORK_G0 = "__NetworkBindableg0";
        public static final String BINDABLE_ID_NETWORK_G1 = "__NetworkBindableg1";
        public static final String BINDABLE_ID_VIEW = "__NetworkBindableView";
        
        private void __init_database_table(CyTableManager tablemgr, CyTableFactory table_fact) {
                Set<CyTable> tables = tablemgr.getGlobalTables();
                CyTable glb_table = null;
                for (CyTable table : tables) {
                        if (table.getTitle().equals(NETWORKDB_TABLE)) {
                                glb_table = table;
                                break;
                        }
                }
                if (glb_table == null) {
                        // The table has not existed yet.
                        m_dbtable = table_fact.createTable(NETWORKDB_TABLE, c_PrimaryKey, Long.class, true, true);
                        m_dbtable.createColumn(c_AttriNetworkName, String.class, true);
                        m_dbtable.createColumn(c_AttriNetworkG0, Long.class, true);
                        m_dbtable.createColumn(c_AttriNetworkG1, Long.class, true);
                        m_dbtable.createColumn(c_AttriNetworkView, Long.class, true);
                }
        }
        
        private void __load_database() {
                List<CyRow> rows = m_dbtable.getAllRows();
                for (CyRow row : rows) {
                        Long network_uuid = row.get(c_PrimaryKey, Long.class);
                        String network_name = row.get(c_AttriNetworkName, String.class);
                        Long networkg0_uuid = row.get(c_AttriNetworkG0, Long.class);
                        Long networkg1_uuid = row.get(c_AttriNetworkG1, Long.class);
                        Long networkview_uuid = row.get(c_AttriNetworkView, Long.class);
                        
                        CyNetwork net, netg0 = null, netg1 = null;
                        CyNetworkView netview = null;
                        if (network_uuid != null) net = m_network_mgr.getNetwork(network_uuid);
                        else {
                                System.out.println(getClass() + " - " + "Such network " + network_name + " doesn't exists.");
                                continue;
                        }
                        if (networkg0_uuid != null) netg0 = m_network_mgr.getNetwork(networkg0_uuid);
                        if (networkg1_uuid != null) netg1 = m_network_mgr.getNetwork(networkg1_uuid);
                        if (networkview_uuid != null) netview = m_netview_mgr.getNetworkViews(net).iterator().next();
                        
                        AlignmentNetwork align_net = new AlignmentNetwork(net);
                        add_network_bindings(align_net, new Bindable(align_net, BINDABLE_ID_NETWORK));
                        if (netg0 != null) 
                                add_network_bindings(align_net, new Bindable(new AlignmentNetwork(netg0), BINDABLE_ID_NETWORK_G0));
                        if (netg1 != null) 
                                add_network_bindings(align_net, new Bindable(new AlignmentNetwork(netg1), BINDABLE_ID_NETWORK_G1));
                        if (netview != null)
                                add_network_bindings(align_net, new Bindable(netview, BINDABLE_ID_VIEW));
                }
        }
        
        private void __store_database() {
                ArrayList<Long> obsolete_uuids = new ArrayList<>();
                
                OUTTER_LOOP:
                for (Long netuuid : m_bindings.keySet()) {
                        Long network_uuid = netuuid;
                        String network_name = "";
                        Long networkg0_uuid = null;
                        Long networkg1_uuid = null;
                        Long networkview_uuid = null;
                        
                        Set<Bindable> bindables = m_bindings.get(netuuid);
                        for (Bindable bindable : bindables) {
                                switch (bindable.get_id()) {
                                        case BINDABLE_ID_NETWORK:
                                                try {
                                                        network_name = ((AlignmentNetwork) bindable.get_binded()).get_suggested_name();
                                                } catch (NullPointerException ex) {
                                                        System.out.println(getClass() + " - " + "Network " + network_uuid 
                                                                        + " has been destroyed, refreshing bindings");
                                                        obsolete_uuids.add(network_uuid);
                                                        continue OUTTER_LOOP;
                                                }
                                                break;
                                        case BINDABLE_ID_NETWORK_G0:
                                                networkg0_uuid = ((AlignmentNetwork) bindable.get_binded()).get_suid();
                                                break;
                                        case BINDABLE_ID_NETWORK_G1:
                                                networkg1_uuid = ((AlignmentNetwork) bindable.get_binded()).get_suid();
                                                break;
                                        case BINDABLE_ID_VIEW:
                                                networkview_uuid = ((CyNetworkView) bindable.get_binded()).getSUID();
                                                break;
                                }
                        }
                        
                        CyRow row = m_dbtable.getRow(network_uuid);
                        row.set(c_PrimaryKey, network_uuid);
                        row.set(c_AttriNetworkName, network_name);
                        row.set(c_AttriNetworkG0, networkg0_uuid);
                        row.set(c_AttriNetworkG1, networkg1_uuid);
                        row.set(c_AttriNetworkView, networkview_uuid);
                }
                
                for (Long obselete_uuid : obsolete_uuids) {
                        m_bindings.remove(obselete_uuid);
                }
        }

        public NetworkDatabase(CySwingAppAdapter app_adapter) {
                m_bindings = new HashMap<>();
                m_network_mgr = app_adapter.getCyNetworkManager();
                m_netview_mgr = app_adapter.getCyNetworkViewManager();
                __init_database_table(app_adapter.getCyTableManager(), app_adapter.getCyTableFactory());
                __load_database();
        }

        public void add_network_bindings(AlignmentNetwork network, Bindable bindable) {
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
        
        public void update() {
                __store_database();
        }

        public Bindable get_network_binding(AlignmentNetwork network, String bindable_id) {
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
