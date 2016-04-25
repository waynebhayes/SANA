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
import java.awt.event.ActionListener;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.util.Arrays;
import java.util.LinkedList;
import java.util.List;
import java.util.Set;
import java.util.logging.Level;
import java.util.logging.Logger;
import org.cytoscape.app.swing.CySwingAppAdapter;
import org.cytoscape.model.CyNetwork;
import org.cytoscape.work.Task;
import org.cytoscape.work.TaskIterator;
import research.TaskSwitchAlignmentView.SwitchMode;

class ReturnValue {
        
        public boolean is_valid = true;
        public List<String> g0_sig = new LinkedList<>();
        public List<String> g1_sig = new LinkedList<>();
        public boolean is_2switch = true;
        public boolean is_2show_neighbour = true;
        public SwitchMode switch_mode = SwitchMode.ShowOrHideAlignedNetwork;
        public String network_selected = "";
}

class AlignmentSwitchInput extends CustomDialog {

        private final CySwingAppAdapter m_adapter;
        private final ReturnValue m_ret = new ReturnValue();
        private final UIGetSwitchNodes m_switch_node_ui = new UIGetSwitchNodes();
        
        class InputDialogListener extends WindowAdapter {
                
                private final AlignmentSwitchInput m_input;
                
                public InputDialogListener(AlignmentSwitchInput input) {
                        m_input = input;
                }
                
                @Override
                public void windowClosing(WindowEvent arg0) {
                        m_input.m_ret.is_valid = false;
                        setVisible(false);
                }
        }
        
        private class ConfirmButtonAction implements ActionListener {
                
                @Override
                public void actionPerformed(ActionEvent e) {
                        setVisible(false);
                }
        }

        public AlignmentSwitchInput(CySwingAppAdapter adapter) {
                m_adapter = adapter;
        }

        @Override
        public void set_data(Object data) {
        }

        private String[] network_set_string() {
                Set<CyNetwork> networks = m_adapter.getCyNetworkManager().getNetworkSet();
                if (!networks.isEmpty()) {
                        String[] list_of_titles = new String[networks.size()];
                        int i = 0;
                        for (CyNetwork network : networks) {
                                list_of_titles[i++] = network.toString();
                        }
                        return list_of_titles;
                } else {
                        return new String[]{"No networks can be shown"};
                }
        }

        /**
         * @return MenuShowAlignmentSwitch.SwitchSpecifier.
         */
        @Override
        public Object get_data() {
                String[] slist;
                slist = m_switch_node_ui.txt_g0_input.getText().split("\n");
                m_ret.g0_sig.addAll(Arrays.asList(slist));
                
                slist = m_switch_node_ui.txt_g1_input.getText().split("\n");
                m_ret.g1_sig.addAll(Arrays.asList(slist));
                
                if (m_switch_node_ui.rb_switch_aligned.isSelected()) {
                        m_ret.switch_mode = SwitchMode.ShowOrHideAlignedNetwork;
                        System.out.println(getClass() + " - " + SwitchMode.ShowOrHideAlignedNetwork);
                } else if (m_switch_node_ui.rb_switch_customized.isSelected()) {
                        m_ret.switch_mode = SwitchMode.ShowOrHideSubnetwork;
                        System.out.println(getClass() + " - " + SwitchMode.ShowOrHideSubnetwork);
                } else {
                        m_ret.switch_mode = SwitchMode.Null;
                        System.out.println(getClass() + " - " + SwitchMode.Null);
                }
                m_ret.is_2switch = m_switch_node_ui.cb_is_2switch.isSelected();
                m_ret.is_2show_neighbour = m_switch_node_ui.cb_2show_neighbour.isSelected();
                return m_ret;
        }

        @Override
        public String get_name() {
                return getClass().toString();
        }

        @Override
        public void run() {
                m_ret.network_selected = Util.run_selection_dialog(network_set_string(),
                                                                   "Available networks",
                                                                   "Please choose a network to switch over");
                m_switch_node_ui.btn_confirm.addActionListener(new ConfirmButtonAction());
                addWindowListener(new InputDialogListener(this));
                setTitle("Alignment Switcher");
                set_container(m_switch_node_ui);
                pack();
        }
}

class DOSUTFFS extends Thread {
        
        private final CytoscapeMenuService    m_service;
        
        public DOSUTFFS(CytoscapeMenuService service) {
                m_service = service;
        }
        
        @Override
        public void run() {
                AlignmentSwitchInput input = new AlignmentSwitchInput(m_service.get_adapter());
                ReturnValue ret = (ReturnValue) Util.run_customized_dialog(input, null);

                if (!ret.is_valid) {
                        System.out.println(getClass() + " - The switch action returns invalid return values. "
                                           + "Stop proceeding");
                        return ;
                }
                
                Task task = new TaskSwitchAlignmentView(m_service.get_adapter(),
                                                        ret.is_2switch,
                                                        ret.switch_mode,
                                                        ret.is_2show_neighbour,
                                                        ret.network_selected,
                                                        ret.g0_sig,
                                                        ret.g1_sig);
                
                CySwingAppAdapter adapter = m_service.get_adapter();
                try {
                        adapter.getTaskManager().execute(new TaskIterator(task));
                        //task.run(null);
                } catch (Exception ex) {
                        Logger.getLogger(DOSUTFFS.class.getName()).log(Level.SEVERE, null, ex);
                }
        }
}

/**
 * Show/hide alignment menu
 *
 * @author davis
 */
public class MenuShowAlignmentSwitch implements MenuProtocol {

        private final String c_MenuName = "Show/Hide Alignment";
        private final String c_ParentMenuName = "Tools";
        private CytoscapeMenuService m_service;

        @Override
        public String get_menu_name() {
                return c_MenuName;
        }

        @Override
        public String get_parent_menu_name() {
                return c_ParentMenuName;
        }

        @Override
        public void action_performed(ActionEvent e) {
                DOSUTFFS dostuffs = new DOSUTFFS(m_service);
                dostuffs.start();
        }

        @Override
        public void set_menu_service(CytoscapeMenuService service) {
                m_service = service;
        }

}
