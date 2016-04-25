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

import java.util.HashSet;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Set;
import org.cytoscape.app.swing.CySwingAppAdapter;
import org.cytoscape.model.CyNetwork;
import org.cytoscape.model.CyNetworkManager;
import org.cytoscape.view.model.CyNetworkView;
import org.cytoscape.work.Task;
import org.cytoscape.work.TaskMonitor;

/**
 * Switch the view of alignment network between show/hidden
 *
 * @author davis
 */
public class TaskSwitchAlignmentView implements Task {

        enum SwitchMode {
                Null,
                ShowOrHideAlignedNetwork,
                ShowOrHideSubnetwork
        }

        private final CySwingAppAdapter m_adapter;
        private final boolean m_is_2switch;
        private final boolean m_2show_neighbour;
        private final SwitchMode m_mode;
        private final List<String> g0_sig;
        private final List<String> g1_sig;
        private final String m_network_selected;

        TaskSwitchAlignmentView(CySwingAppAdapter adapter,
                                boolean is_2switch, 
                                SwitchMode switch_mode,
                                boolean is_2show_neighbour,
                                String network_selected,
                                List<String> g0_sig, List<String> g1_sig) {
                m_adapter = adapter;
                m_is_2switch = is_2switch;
                m_2show_neighbour = is_2show_neighbour;
                m_mode = switch_mode;
                this.g0_sig = g0_sig;
                this.g1_sig = g1_sig;
                m_network_selected = network_selected;
        }

        private CyNetwork get_network_by_name(String network_name) {
                CyNetworkManager mgr = m_adapter.getCyNetworkManager();
                Set<CyNetwork> networks = mgr.getNetworkSet();
                for (CyNetwork network : networks) {
                        if (network.getRow(network).
                                get(CyNetwork.NAME, String.class).
                                equals(network_name)) {
                                return network;
                        }
                }
                return null;
        }

        class Bindings {

                public AlignmentNetwork aligned;
                public AlignmentNetwork g0;
                public AlignmentNetwork g1;
                public CyNetworkView view;
        }

        private Bindings get_bindings_for(String network_name) throws Exception {
                Bindings bindings = new Bindings();

                NetworkDatabase db = NetworkDatabaseSingleton.get_instance();
                CyNetwork network = get_network_by_name(network_name);
                AlignmentNetwork align_net = new AlignmentNetwork(network);
                if (!db.has_network(align_net)) {
                        // Doesn't have record of this network, skip
                        throw new Exception(getClass() + " - doesn't have record of: " + network.getSUID());
                }

                Bindable b_g0_network = db.get_network_binding(align_net, NetworkDatabase.BINDABLE_ID_NETWORK_G0);
                Bindable b_g1_network = db.get_network_binding(align_net, NetworkDatabase.BINDABLE_ID_NETWORK_G1);
                Bindable b_network_view = db.get_network_binding(align_net, NetworkDatabase.BINDABLE_ID_VIEW);
                AlignmentNetwork g0 = (AlignmentNetwork) b_g0_network.get_binded();
                AlignmentNetwork g1 = (AlignmentNetwork) b_g1_network.get_binded();
                CyNetworkView view = (CyNetworkView) b_network_view.get_binded();

                bindings.aligned = align_net;
                bindings.view = view;
                bindings.g0 = g0;
                bindings.g1 = g1;
                return bindings;
        }

        private void show_or_hide_aligned_network(String network_name, TaskMonitor tm) throws Exception {
                System.out.println(getClass() + " - show_or_hide_aligned_network...");
                Bindings bindings = get_bindings_for(network_name);
                // Obtain data from database
                System.out.println(getClass() + " - network: " + m_network_selected
                                   + " is in the database, will modify this network");

                // Configurate and decorate the view
                HashSet<AlignmentNetwork> aligned_set = new HashSet<>();
                aligned_set.add(bindings.g0);
                aligned_set.add(bindings.g1);

                NetworkDescriptor aligned_compl_desc = new NetworkDescriptor(bindings.aligned);
                NetworkDescriptor aligned_desc = new NetworkDescriptor(bindings.aligned);

                aligned_compl_desc.select_by_belongings(aligned_set, true, m_2show_neighbour);
                aligned_desc.select_by_belongings(aligned_set, false, m_2show_neighbour);

                NetworkRenderer renderer = new NetworkRenderer(m_adapter.getCyNetworkViewFactory());
                List<NetworkRenderer.Batch> batches = new LinkedList<>();

                if (m_is_2switch == true) {
                        // Hide unaligned nodes/edges
                        NetworkRenderer.Shader sha_compl = renderer.create_shader(null, 0);
                        NetworkRenderer.Shader sha_align = renderer.create_shader(null, 255);
                        batches.add(renderer.create_batch(aligned_compl_desc, sha_compl));
                        batches.add(renderer.create_batch(aligned_desc, sha_align));
                } else {
                        // Show everything
                        NetworkRenderer.Shader sha_compl = renderer.create_shader(null, 127);
                        batches.add(renderer.create_batch(aligned_compl_desc, sha_compl));
                }

                Map<AlignmentNetwork, CyNetworkView> views = renderer.render(batches, bindings.view, true, tm);
                renderer.commit(views, m_adapter.getCyNetworkViewManager());
                System.out.println(getClass() + " - Finished switching alignment view...");
        }

        private void show_or_hide_subnetwork(String network_name,
                                             TaskMonitor tm) throws Exception {
                System.out.println(getClass() + " - show_or_hide_subnetwork...");
                Bindings bindings = get_bindings_for(network_name);
                // Obtain data from database
                System.out.println(getClass() + " - network: " + m_network_selected
                                   + " is in the database, will modify this network");

                // Configurate and decorate the view
                NetworkRenderer renderer = new NetworkRenderer(m_adapter.getCyNetworkViewFactory());
                List<NetworkRenderer.Batch> batches = new LinkedList<>();

                NetworkDescriptor g0_complement_desc = new NetworkDescriptor(bindings.aligned);
                NetworkDescriptor g1_complement_desc = new NetworkDescriptor(bindings.aligned);
                
                Set<NodeSignatureManager> g0_real_sigs = new HashSet<>();
                for (String plain_sig : g0_sig) {
                        if (plain_sig.equals(""))
                                continue;
                        NodeSignatureManager sig_mgr = new NodeSignatureManager();
                        sig_mgr.add_namespaced_id(bindings.g0.get_network_namespace(), plain_sig);
                        g0_real_sigs.add(sig_mgr);
                }
                Set<NodeSignatureManager> g1_real_sigs = new HashSet<>();
                for (String plain_sig : g1_sig) {
                        if (plain_sig.equals(""))
                                continue;
                        NodeSignatureManager sig_mgr = new NodeSignatureManager();
                        sig_mgr.add_namespaced_id(bindings.g1.get_network_namespace(), plain_sig);
                        g1_real_sigs.add(sig_mgr);
                }
                if (!g0_real_sigs.isEmpty())
                        g0_complement_desc.select_by_complement_signatures(
                                g0_real_sigs, m_2show_neighbour);
                if (!g1_real_sigs.isEmpty())
                        g1_complement_desc.select_by_complement_signatures(
                                g1_real_sigs, m_2show_neighbour);
                
                NetworkRenderer.Shader sha_g0;
                NetworkRenderer.Shader sha_g1;
                if (m_is_2switch == true) {
                        // Hide unaligned nodes/edges
                        sha_g0 = renderer.create_shader(null, 0);
                        sha_g1 = renderer.create_shader(null, 0);
                } else {
                        // Show everything
                        sha_g0 = renderer.create_shader(null, 127);
                        sha_g1 = renderer.create_shader(null, 127);
                }
                batches.add(renderer.create_batch(g0_complement_desc, sha_g0));
                batches.add(renderer.create_batch(g1_complement_desc, sha_g1));
                        
                Map<AlignmentNetwork, CyNetworkView> views = renderer.render(batches, bindings.view, true, tm);
                renderer.commit(views, m_adapter.getCyNetworkViewManager());

                System.out.println(getClass() + " - Finished switching alignment view...");
        }

        @Override
        public void run(TaskMonitor tm) throws Exception {
                tm.setTitle("Switching Network View");

                switch (m_mode) {
                        case ShowOrHideAlignedNetwork:
                                show_or_hide_aligned_network(m_network_selected, tm);
                                break;
                        case ShowOrHideSubnetwork:
                                show_or_hide_subnetwork(m_network_selected, tm);
                                break;
                }
        }

        @Override
        public void cancel() {
                // do nothing
        }
}
