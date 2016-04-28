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
import java.util.Collection;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Set;
import org.cytoscape.model.CyEdge;
import org.cytoscape.model.CyNode;
import org.cytoscape.view.model.CyNetworkView;
import org.cytoscape.view.model.CyNetworkViewFactory;
import org.cytoscape.view.model.CyNetworkViewManager;
import org.cytoscape.view.model.View;
import org.cytoscape.view.presentation.property.BasicVisualLexicon;
import org.cytoscape.view.presentation.property.NodeShapeVisualProperty;
import org.cytoscape.view.presentation.property.values.NodeShape;
import org.cytoscape.work.TaskMonitor;

/**
 * Facility to render the network.
 * @author davis
 */
public class NetworkRenderer {
        public class Shader {
                public final Color m_color;
                public final Integer m_transparency;
                public final Boolean m_2show_label;
                
                public Shader(Color c, Integer t, Boolean show_label) {
                        m_color = c;
                        m_transparency = t;
                        m_2show_label = show_label;
                }
        }
        
        public class Batch {
                public final NetworkDescriptor m_desc;
                public final Shader m_shader;
                
                public Batch(NetworkDescriptor desc, Shader shader) {
                        m_desc = desc;
                        m_shader = shader;
                }
        }
        
        private final CyNetworkViewFactory m_view_fact;
        private final Double c_NodeWidth = 12.0;
        private final Double c_NodeHeight = 12.0;
        private final Double c_NodeEnlargedWidth = 50.0;
        private final Double c_NodeEnlargedHeight = 50.0;
        private final NodeShape c_NodeShape = NodeShapeVisualProperty.ELLIPSE;
        
        public NetworkRenderer(CyNetworkViewFactory view_fact) {
                m_view_fact = view_fact;
        }
        
//        public NetworkRenderer() {
//                m_view_fact = null;
//        }
        
        public Shader create_shader(Color c, Integer t, boolean show_label) {
                return new Shader(c, t, show_label);
        }
        
        public Shader create_shader(Color c, Integer t) {
                return new Shader(c, t, null);
        }
        
        public Shader create_shader(boolean show_label) {
                return new Shader(null, null, show_label);
        }
        
        public Batch create_batch(NetworkDescriptor desc, Shader shader) {
                return new Batch(desc, shader);
        }
        
        public Map<AlignmentNetwork, CyNetworkView> render(List<Batch> batches, CyNetworkView custom_view, boolean render2custom, TaskMonitor tm) {
                if (custom_view == null && m_view_fact == null) {
                        return null;
                }
                // Keep track of views
                HashMap<AlignmentNetwork, CyNetworkView> uniqueness = new HashMap<>();

                // process batches
                for (Batch batch : batches) {
                        NetworkDescriptor desc = batch.m_desc;
                        AlignmentNetwork align_net = desc.get_alignment_network();
                        
                        CyNetworkView view;
                        if (render2custom && custom_view != null) {
                                view = custom_view;
                        } else {
                                view = uniqueness.get(align_net);
                                if (view == null) {
                                        view = m_view_fact.createNetworkView(align_net.get_network());
                                        uniqueness.put(align_net, view);
                                }
                        }

                        tm.setTitle("Rendering Alignemnt Network");

                        // To keep track of progress
                        tm.setStatusMessage("Rendering " + align_net.get_suggested_name());
                        int j = 0;
                        int total = align_net.get_node_count() + align_net.get_edge_count();

                        // Decorate nodes
                        NodeSignatureManager sig_mgr = new NodeSignatureManager();
                        Set<CyNode> net_nodes = desc.get_network_nodes();
                        List<NodeSignatureManager> net_node_sigs = desc.get_network_node_signatures();
                        Shader shader = batch.m_shader;
                        int k = 0;
                        for (CyNode node : net_nodes) {
                                NodeSignatureManager sigs = net_node_sigs.get(k ++);
                                String formal_name = sigs.toString();
                                String shortened_name = sigs.toSimplifiedString();
                                View<CyNode> node_view = view.getNodeView(node);
                                View<CyNode> custom_node_view = custom_view != null ? custom_view.getNodeView(node) : null;
                                node_view.setVisualProperty(BasicVisualLexicon.NODE_WIDTH, c_NodeWidth);
                                node_view.setVisualProperty(BasicVisualLexicon.NODE_HEIGHT, c_NodeHeight);
                                node_view.setVisualProperty(BasicVisualLexicon.NODE_BORDER_WIDTH, 0.0);
                                node_view.setVisualProperty(BasicVisualLexicon.NODE_SHAPE, c_NodeShape);
                                node_view.setVisualProperty(BasicVisualLexicon.NODE_TOOLTIP, formal_name);
                                if (shader != null) {
                                        if (shader.m_color != null)
                                                node_view.setVisualProperty(BasicVisualLexicon.NODE_FILL_COLOR, 
                                                                            shader.m_color);
                                        else {
                                                if (custom_node_view != null) {
                                                        // Use custom view's property if possible.
                                                        node_view.setVisualProperty(BasicVisualLexicon.NODE_FILL_COLOR, 
                                                                custom_node_view.getVisualProperty(BasicVisualLexicon.NODE_FILL_COLOR));
                                                }
                                        }
                                        if (shader.m_transparency != null)
                                                node_view.setVisualProperty(BasicVisualLexicon.NODE_TRANSPARENCY, 
                                                                            shader.m_transparency);
                                        else {
                                                if (custom_node_view != null) {
                                                        // Use custom view's property if possible.
                                                        node_view.setVisualProperty(BasicVisualLexicon.NODE_TRANSPARENCY, 
                                                                custom_node_view.getVisualProperty(BasicVisualLexicon.NODE_TRANSPARENCY));
                                                }
                                        }
                                        if (shader.m_2show_label != null && shader.m_2show_label == true) {
                                                node_view.setVisualProperty(BasicVisualLexicon.NODE_WIDTH, c_NodeEnlargedWidth);
                                                node_view.setVisualProperty(BasicVisualLexicon.NODE_HEIGHT, c_NodeEnlargedHeight);
                                                node_view.setVisualProperty(BasicVisualLexicon.NODE_LABEL, shortened_name);
                                                node_view.setVisualProperty(BasicVisualLexicon.NODE_LABEL_FONT_SIZE, 14);
                                        } else {
                                                node_view.setVisualProperty(BasicVisualLexicon.NODE_WIDTH, c_NodeWidth);
                                                node_view.setVisualProperty(BasicVisualLexicon.NODE_HEIGHT, c_NodeHeight);
                                                node_view.setVisualProperty(BasicVisualLexicon.NODE_LABEL, null);
                                        }
                                }
                                Util.advance_progress(tm, j, total);
                        }
                        // Decorate edges
                        Set<CyEdge> net_edges = desc.get_network_edges();
                        for (CyEdge edge : net_edges) {
                                View<CyEdge> edge_view = view.getEdgeView(edge);
                                if (shader != null) {
                                        if (shader.m_color != null)
                                                edge_view.setVisualProperty(
                                                        BasicVisualLexicon.EDGE_STROKE_UNSELECTED_PAINT, 
                                                        shader.m_color);
                                        if (shader.m_transparency != null)
                                                edge_view.setVisualProperty(
                                                        BasicVisualLexicon.EDGE_TRANSPARENCY, 
                                                        shader.m_transparency);
                                }
                                Util.advance_progress(tm, j, total);
                        }
                }
                return uniqueness;
        }
        
        public Map<AlignmentNetwork, CyNetworkView> render(List<Batch> batches) {
                return render(batches, null, false, null);
        }
        
        public Map<AlignmentNetwork, CyNetworkView> render(List<Batch> batches, CyNetworkView view) {
                return render(batches, view, true, null);
        }
        
        public void commit(Map<AlignmentNetwork, CyNetworkView> views, CyNetworkViewManager view_mgr) {
                // Clear old views
                for (AlignmentNetwork align_net : views.keySet()) {
                        CyNetworkView new_view = views.get(align_net);
                        if (view_mgr.viewExists(align_net.get_network())) {
                                Collection<CyNetworkView> old_views = view_mgr.getNetworkViews(align_net.get_network());
                                for (CyNetworkView old_view : old_views) {
                                        if (old_view != new_view)
                                                view_mgr.destroyNetworkView(old_view);
                                }
                        }
                }
                // Add new views
                for (CyNetworkView view : views.values()) {
                        view_mgr.addNetworkView(view);
                }
                // update network view
                for (CyNetworkView view : views.values()) {
                        view.updateView();
                }
        }
}
