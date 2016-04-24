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
import java.util.Properties;
import org.cytoscape.app.swing.CySwingAppAdapter;
import org.cytoscape.application.swing.AbstractCyAction;
import org.cytoscape.application.swing.CyAction;

class MenuItemAction extends AbstractCyAction {

        private MenuProtocol m_protocol = null;
        private CytoscapeMenuService m_service = null;

        public MenuItemAction(MenuProtocol protocol, CytoscapeMenuService service) {
                super(protocol.get_menu_name());
                setPreferredMenu(protocol.get_parent_menu_name());
                protocol.set_menu_service(service);
                m_protocol = protocol;
                m_service = service;
        }

        @Override
        public void actionPerformed(ActionEvent e) {
                m_protocol.action_performed(e);
        }
}

/**
 * To install menus to cytoscape.
 *
 * @author Wen, Chifeng <https://sourceforge.net/u/daviesx/profile/>
 */
public class CytoscapeMenuService {

        private CySwingAppAdapter m_adapter = null;

        CytoscapeMenuService(CySwingAppAdapter adapter) {
                m_adapter = adapter;
        }

        public boolean install_protocol(MenuProtocol protocol) {
                System.out.println(getClass() + " - Installing menu protocol: " + protocol + "...");
                MenuItemAction action = new MenuItemAction(protocol, this);
                Properties props = new Properties();
                m_adapter.getCyServiceRegistrar().registerService(action, CyAction.class, props);
                return true;
        }

        CySwingAppAdapter get_adapter() {
                return m_adapter;
        }
}
