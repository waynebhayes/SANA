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
import java.util.Map;
import org.cytoscape.app.swing.CySwingAppAdapter;
import org.cytoscape.model.CyNetwork;
import org.cytoscape.view.model.CyNetworkView;
import org.cytoscape.work.Task;
import org.cytoscape.work.TaskMonitor;

/**
 * @author davis
 */
public class TaskShowLabel implements Task {
        private final CySwingAppAdapter m_adapter;
        private final boolean m_is2show;
        
        public TaskShowLabel(boolean is2show, CySwingAppAdapter adapter) {
                m_adapter = adapter;
                m_is2show = is2show;
        }

        @Override
        public void run(TaskMonitor tm) throws Exception {
                NetworkDatabase db = NetworkDatabaseSingleton.get_instance();
                CyNetwork curr_net = m_adapter.getCyApplicationManager().getCurrentNetwork();
                
                AlignmentNetwork aligned_net = new AlignmentNetwork(curr_net);
                CyNetworkView view = (CyNetworkView) db.get_network_binding(aligned_net, NetworkDatabase.BINDABLE_ID_VIEW).get_binded();
                AlignmentNetwork g0 = (AlignmentNetwork) db.get_network_binding(aligned_net, NetworkDatabase.BINDABLE_ID_NETWORK_G0).get_binded();
                AlignmentNetwork g1 = (AlignmentNetwork) db.get_network_binding(aligned_net, NetworkDatabase.BINDABLE_ID_NETWORK_G1).get_binded();
                
                if (m_is2show)
                        System.out.println(getClass().toString() + " - Show lbel for network: " 
                                           + aligned_net.get_suggested_name() + ", with view: " + view);
                else
                        System.out.println(getClass().toString() + " - Hide label for network: " 
                                           + aligned_net.get_suggested_name() + ", with view: " + view);
                
                NetworkDescriptor desc = new NetworkDescriptor(aligned_net);
                desc.select_all();
                
                NetworkRenderer renderer = new NetworkRenderer(m_adapter.getCyNetworkViewFactory());
                NetworkRenderer.Shader label_sha = renderer.create_shader(m_is2show);
                List<NetworkRenderer.Batch> batches = new ArrayList<>();
                batches.add(renderer.create_batch(desc, label_sha));
                
                Map<AlignmentNetwork, CyNetworkView> views = renderer.render(batches, view, true, tm);
                renderer.commit(views, m_adapter.getCyNetworkViewManager());
        }

        @Override
        public void cancel() {
        }
}
