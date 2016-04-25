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
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.util.Set;
import java.util.logging.Level;
import java.util.logging.Logger;
import org.cytoscape.app.swing.CySwingAppAdapter;
import org.cytoscape.model.CyTable;
import org.cytoscape.work.Task;
import org.cytoscape.work.TaskIterator;
import org.cytoscape.work.TaskMonitor;

class LoadAlignmentDataTask implements Task {

        private final LoaderSANAAlign m_sana_align;
        private final CySwingAppAdapter m_adapter;

        LoadAlignmentDataTask(LoaderSANAAlign sana_align, CySwingAppAdapter adapter) {
                m_sana_align = sana_align;
                m_adapter = adapter;
        }

        @Override
        public void run(TaskMonitor tm) throws Exception {
                m_sana_align.run(tm);
                CyTable[] tables = m_sana_align.getTables();
                for (CyTable table : tables) {
                        m_adapter.getCyTableManager().addTable(table);
                }
        }

        @Override
        public void cancel() {
                m_sana_align.cancel();
        }

}

/**
 * Menu Item for importing alignment file. Because Cytoscape couldn't load
 * alignment data as table correctly, we will have to do it manually
 *
 * @author Wen, Chifeng <https://sourceforge.net/u/daviesx/profile/>
 */
public class MenuImportAlignmentFile implements MenuProtocol {

        private final String c_MenuName = "SANA Alignment File";
        private final String c_ParentMenuName = "File.Import";

        private CytoscapeMenuService m_service = null;
        private CySwingAppAdapter m_adapter = null;

        private final LoaderSANAAlign m_sana_align;

        MenuImportAlignmentFile(LoaderSANAAlign sana_align) {
                m_sana_align = sana_align;
        }

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
                // Manually take the task to import the alignment data
                Set<String> ext = m_sana_align.get_file_extension();
                File file = Util.run_file_chooser(m_sana_align.get_file_description(),
                                                  ext.iterator().next());
                if (file != null) {
                        try {
                                FileInputStream stream = new FileInputStream(file.getPath());
                                m_sana_align.set_file_name(file.getName());
                                m_sana_align.set_input_stream(stream);
                                m_sana_align.set_loader_service(new CytoscapeLoaderService(m_adapter));
                                m_sana_align.override_data_name(file.getName());
                                // Build a task to load the alignment
                                LoadAlignmentDataTask task = new LoadAlignmentDataTask(m_sana_align, m_adapter);
                                m_adapter.getTaskManager().execute(new TaskIterator(task));
                        } catch (FileNotFoundException ex) {
                                Util.prompt_critical_error(getClass().toString(), ex.getMessage());
                                Logger.getLogger(MenuImportAlignmentFile.class.getName()).log(Level.SEVERE, null, ex);
                        }
                } else {
                        Util.prompt_critical_error(getClass().toString(), "Path to alignment data is not given");
                }
        }

        @Override
        public void set_menu_service(CytoscapeMenuService service) {
                m_service = service;
                m_adapter = m_service.get_adapter();
        }

}
