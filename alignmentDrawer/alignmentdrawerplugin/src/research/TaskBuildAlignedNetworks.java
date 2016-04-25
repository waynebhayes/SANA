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

import java.awt.Color;
import java.util.HashSet;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Set;
import org.cytoscape.app.swing.CySwingAppAdapter;
import org.cytoscape.model.CyNetwork;
import org.cytoscape.model.CyNetworkManager;
import org.cytoscape.model.CyTable;
import org.cytoscape.view.model.CyNetworkView;
import org.cytoscape.view.model.CyNetworkViewManager;
import org.cytoscape.work.Task;
import org.cytoscape.work.TaskMonitor;

/**
 * A task to compute and build an aligned network.
 *
 * @author Wen, Chifeng <https://sourceforge.net/u/daviesx/profile/>
 */
public class TaskBuildAlignedNetworks implements Task {

        private final CySwingAppAdapter m_adapter;
        private final CyTable m_data;
        private final CyNetwork m_g0;
        private final CyNetwork m_g1;

        TaskBuildAlignedNetworks(CyTable align_data, CyNetwork g0, CyNetwork g1,
                                 CySwingAppAdapter adapter) {
                m_adapter = adapter;
                m_data = align_data;
                m_g0 = g0;
                m_g1 = g1;
        }

        @Override
        public void run(TaskMonitor tm) throws Exception {
                System.out.println(getClass() + " - Start aligning networks...");

                tm.setTitle("Aligning networks...");
                tm.setStatusMessage("Aligning g0 with g1...");
                tm.setProgress(0.0);

                // Align the networks
                NetworkAligner aligner = new NetworkAligner(m_data);
                AlignmentNetwork g0 = new AlignmentNetwork(m_g0);
                AlignmentNetwork g1 = new AlignmentNetwork(m_g1);
                CyNetwork cyaligned = m_adapter.getCyNetworkFactory().createNetwork();
                AlignmentNetwork aligned = new AlignmentNetwork(cyaligned);
                aligner.align_networks_from_data(g0, g1, aligned);

                // Configure the style
                tm.setStatusMessage("Styling the aligned network...");
                // @todo: coloring scheme is hardcoded, maybe better find a way to let user choose the scheme
                NetworkDescriptor g0_desc = new NetworkDescriptor(aligned);
                NetworkDescriptor g1_desc = new NetworkDescriptor(aligned);
                NetworkDescriptor g01_desc = new NetworkDescriptor(aligned);
                
                Set<AlignmentNetwork> aligned_set = new HashSet<>();
                Set<AlignmentNetwork> g0_set = new HashSet<>();
                Set<AlignmentNetwork> g1_set = new HashSet<>();
                
                aligned_set.add(g0);
                aligned_set.add(g1);
                g0_set.add(g0);
                g1_set.add(g1);
                
                g0_desc.select_by_belongings(g0_set);
                g1_desc.select_by_belongings(g1_set);
                g01_desc.select_by_belongings(aligned_set);
                
                NetworkRenderer renderer = new NetworkRenderer(m_adapter.getCyNetworkViewFactory());
                NetworkRenderer.Shader sha_aligned = renderer.create_shader(Color.GREEN, 255);
                NetworkRenderer.Shader sha_g0 = renderer.create_shader(Color.RED, 127);
                NetworkRenderer.Shader sha_g1 = renderer.create_shader(Color.BLACK, 127);

                List<NetworkRenderer.Batch> batches = new LinkedList<>();
                batches.add(renderer.create_batch(g0_desc, sha_g0));
                batches.add(renderer.create_batch(g1_desc, sha_g1));
                batches.add(renderer.create_batch(g01_desc, sha_aligned));
                
                // Apply visual style to the network
                // Then put new network and its view to the manager
                CyNetworkManager network_mgr = m_adapter.getCyNetworkManager();
                CyNetworkViewManager view_mgr = m_adapter.getCyNetworkViewManager();
                
                network_mgr.addNetwork(aligned.get_network());
                
                Map<AlignmentNetwork, CyNetworkView> views = renderer.render(batches, null, false, tm);
                renderer.commit(views, view_mgr);

                // Put the aligned network to local database
                NetworkDatabase db = NetworkDatabaseSingleton.get_instance();
                db.add_network_bindings(aligned, new Bindable(aligned, NetworkDatabase.BINDABLE_ID_NETWORK));
                
                for (CyNetworkView view : views.values()) {
                        db.add_network_bindings(aligned, new Bindable(view, NetworkDatabase.BINDABLE_ID_VIEW));
                }
                
                db.add_network_bindings(aligned, new Bindable(g0, NetworkDatabase.BINDABLE_ID_NETWORK_G0));
                db.add_network_bindings(aligned, new Bindable(g1, NetworkDatabase.BINDABLE_ID_NETWORK_G1));
                db.update();

                tm.setProgress(1.0);
                tm.setStatusMessage("Finished aligning networks...");
                System.out.println(getClass() + " - Finished aligning networks...");
        }

        @Override
        public void cancel() {
        }
}
