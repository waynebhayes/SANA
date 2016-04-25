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

import java.awt.event.ActionEvent;
import org.cytoscape.app.swing.CySwingAppAdapter;
import org.cytoscape.work.TaskIterator;

/**
 * @author Wen, Chifeng
 */
public class MenuShowLabel implements MenuProtocol {
        
        private CytoscapeMenuService m_service = null;
        private CySwingAppAdapter m_adapter = null;
        private boolean m_is2show = false;
        
        @Override
        public String get_menu_name() {
                return "Show/Hide Label";
        }

        @Override
        public String get_parent_menu_name() {
                return "Tools";
        }

        @Override
        public void action_performed(ActionEvent e) {
                m_is2show = !m_is2show;
                TaskShowLabel task = new TaskShowLabel(m_is2show, m_adapter);
                m_adapter.getTaskManager().execute(new TaskIterator(task));
        }

        @Override
        public void set_menu_service(CytoscapeMenuService service) {
                m_service = service;
                m_adapter = m_service.get_adapter();
        }      
}
