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

import java.awt.Color;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;
import org.cytoscape.app.swing.CySwingAppAdapter;
import org.cytoscape.model.CyNetwork;
import org.cytoscape.view.model.CyNetworkView;
import org.cytoscape.work.Task;
import org.cytoscape.work.TaskMonitor;

/**
 * @author davis
 */
public class TaskHighlightNode implements Task {
        private final String m_sig;
        private final CySwingAppAdapter m_adapter;
        
        public TaskHighlightNode(String sig, CySwingAppAdapter adapter) {
                m_sig = sig;
                m_adapter = adapter;
        }

        @Override
        public void run(TaskMonitor tm) throws Exception {
                if (m_sig == null || "".equals(m_sig)) {
                        System.out.println(getClass().toString() + " - node name is not specified");
                        return ;
                }
                
                NetworkDatabase db = NetworkDatabaseSingleton.get_instance();
                CyNetwork curr_net = m_adapter.getCyApplicationManager().getCurrentNetwork();
                if (curr_net == null)
                        throw new Exception("You don't have any active network yet");
                
                AlignmentNetwork aligned_net = new AlignmentNetwork(curr_net);
                CyNetworkView view = (CyNetworkView) db.get_network_binding(aligned_net, NetworkDatabase.BINDABLE_ID_VIEW).get_binded();
                AlignmentNetwork g0 = (AlignmentNetwork) db.get_network_binding(aligned_net, NetworkDatabase.BINDABLE_ID_NETWORK_G0).get_binded();
                AlignmentNetwork g1 = (AlignmentNetwork) db.get_network_binding(aligned_net, NetworkDatabase.BINDABLE_ID_NETWORK_G1).get_binded();
                
                System.out.println(getClass() + " - Highlighting network: " + aligned_net.get_suggested_name() + 
                                   ", with view: " + view + ", with node: " + m_sig);
                
                NetworkDescriptor desc = new NetworkDescriptor(aligned_net);
                Set highlight_set = new HashSet();
                NodeSignatureManager sig_mgr0 = new NodeSignatureManager();
                sig_mgr0.add_namespaced_id(g0.get_network_namespace(), m_sig);
                sig_mgr0.add_namespaced_id(g1.get_network_namespace(), m_sig);
                
                NodeSignatureManager sig_mgr1 = new NodeSignatureManager();
                sig_mgr1.add_namespaced_id(g0.get_network_namespace(), m_sig);
                
                NodeSignatureManager sig_mgr2 = new NodeSignatureManager();
                sig_mgr2.add_namespaced_id(g1.get_network_namespace(), m_sig);
                
                highlight_set.add(sig_mgr0);
                highlight_set.add(sig_mgr1);
                highlight_set.add(sig_mgr2);
                desc.select_by_signatures(highlight_set);
                
                NetworkRenderer renderer = new NetworkRenderer(m_adapter.getCyNetworkViewFactory());
                NetworkRenderer.Shader highlight_sha = renderer.create_shader(Color.MAGENTA, 255);
                List<NetworkRenderer.Batch> batches = new ArrayList<>();
                batches.add(renderer.create_batch(desc, highlight_sha));
                
                Map<AlignmentNetwork, CyNetworkView> views = renderer.render(batches, view, true, tm);
                renderer.commit(views, m_adapter.getCyNetworkViewManager());
        }

        @Override
        public void cancel() {
        }
}
