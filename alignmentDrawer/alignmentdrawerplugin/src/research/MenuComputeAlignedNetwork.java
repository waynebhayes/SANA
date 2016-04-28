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

import java.awt.event.ActionEvent;
import java.util.Set;
import org.cytoscape.app.swing.CySwingAppAdapter;
import org.cytoscape.model.CyNetwork;
import org.cytoscape.model.CyTable;
import org.cytoscape.work.TaskIterator;

/**
 * Compute Aligned Network Menu Item (control)
 *
 * @author Wen, Chifeng <https://sourceforge.net/u/daviesx/profile/>
 */
public class MenuComputeAlignedNetwork implements MenuProtocol {

        final private String c_MenuName = "Compute Aligned Networks";
        final private String c_ParentMenuName = "Tools";

        private CytoscapeMenuService m_service = null;
        private CySwingAppAdapter m_adapter = null;

        MenuComputeAlignedNetwork() {
        }

        @Override
        public String get_menu_name() {
                return c_MenuName;
        }

        @Override
        public String get_parent_menu_name() {
                return c_ParentMenuName;
        }

        private Set<CyTable> get_alignment_data_set() {
                return m_adapter.getCyTableManager().getAllTables(true);
        }

        private String[] alignment_data_set_string() {
                Set<CyTable> tables = get_alignment_data_set();
                if (!tables.isEmpty()) {
                        String[] list_of_titles = new String[tables.size()];
                        int i = 0;
                        for (CyTable table : tables) {
                                list_of_titles[i++] = table.getTitle();
                        }
                        return list_of_titles;
                } else {
                        return new String[]{""};
                }
        }

        private CyTable get_selected_data(String selection) {
                if (selection == null) {
                        return null;
                }
                Set<CyTable> tables = get_alignment_data_set();
                for (CyTable table : tables) {
                        if (table.getTitle().equals(selection)) {
                                return table;
                        }
                }
                return null;
        }

        private Set<CyNetwork> get_network_set() {
                return m_adapter.getCyNetworkManager().getNetworkSet();
        }

        private String[] network_set_string() {
                Set<CyNetwork> networks = get_network_set();
                if (!networks.isEmpty()) {
                        String[] list_of_titles = new String[networks.size()];
                        int i = 0;
                        for (CyNetwork network : networks) {
                                list_of_titles[i++] = network.toString();
                        }
                        return list_of_titles;
                } else {
                        return new String[]{""};
                }
        }

        private CyNetwork get_selected_network(String selection) {
                if (selection == null) {
                        return null;
                }
                Set<CyNetwork> networks = get_network_set();
                for (CyNetwork network : networks) {
                        if (network.toString().equals(selection)) {
                                return network;
                        }
                }
                return null;
        }

        @Override
        public void action_performed(ActionEvent e) {
                System.out.println(getClass() + " - " + e.getActionCommand());
                // Get and alignment data from user's choice and build network aligner
                String align_sig = Util.run_selection_dialog(alignment_data_set_string(),
                                                             "Available alignment data",
                                                             "Please choose from the following alignment data: ");
                CyTable data = get_selected_data(align_sig);
                if (data == null) {
                        Util.prompt_critical_error(getClass().toString(), "No alignment data is selected");
                        return;
                }
                // Get networks from user's choice
                String g0 = Util.run_selection_dialog(network_set_string(),
                                                      "Available networks",
                                                      "Please choose g0 from the following network");
                String g1 = Util.run_selection_dialog(network_set_string(),
                                                      "Available networks",
                                                      "Please choose g1 from the following network");
                CyNetwork cyg0 = get_selected_network(g0);
                CyNetwork cyg1 = get_selected_network(g1);
                if (cyg0 == null) {
                        Util.prompt_critical_error(getClass().toString(), "G0 network is invalid");
                        return;
                }
                if (cyg1 == null) {
                        Util.prompt_critical_error(getClass().toString(), "G1 network is invalid");
                        return;
                }
                // Build a task to do the alignment
                TaskBuildAlignedNetworks task = new TaskBuildAlignedNetworks(data, cyg0, cyg1, m_adapter);
                m_adapter.getTaskManager().execute(new TaskIterator(task));
        }

        @Override
        public void set_menu_service(CytoscapeMenuService service) {
                m_service = service;
                m_adapter = m_service.get_adapter();
        }
}
