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

import java.io.InputStream;
import java.util.Properties;
import org.cytoscape.app.swing.CySwingAppAdapter;
import org.cytoscape.io.BasicCyFileFilter;
import org.cytoscape.io.CyFileFilter;
import org.cytoscape.io.read.InputStreamTaskFactory;
import org.cytoscape.model.CyNetworkFactory;
import org.cytoscape.model.CyTableFactory;
import org.cytoscape.view.model.CyNetworkViewFactory;
import org.cytoscape.work.TaskIterator;

/**
 * Direct the InputStream to our LoaderProtocol in order to create a network.
 *
 * @author Wen, Chifeng <https://sourceforge.net/u/daviesx/profile/>
 */
class LoaderInputStreamFactory implements InputStreamTaskFactory {

        private final BasicCyFileFilter m_filter;
        private final LoaderProtocol m_protocol;
        private final CytoscapeLoaderService m_service;

        public LoaderInputStreamFactory(LoaderProtocol protocol,
                                        BasicCyFileFilter filter,
                                        CytoscapeLoaderService service) {
                m_filter = filter;
                m_service = service;
                m_protocol = protocol;
        }

        @Override
        public TaskIterator createTaskIterator(InputStream in, String string) {
                m_protocol.set_file_name(string);
                m_protocol.set_input_stream(in);
                m_protocol.set_loader_service(m_service);
                return new TaskIterator(m_protocol);
        }

        @Override
        public boolean isReady(InputStream in, String string) {
                return true;
        }

        @Override
        public CyFileFilter getFileFilter() {
                return m_filter;
        }
}

/**
 * Create a loader service with given LoaderProtocol.
 *
 * @author Wen, Chifeng <https://sourceforge.net/u/daviesx/profile/>
 */
public class CytoscapeLoaderService {

        private final CySwingAppAdapter m_adapter;

        public CytoscapeLoaderService(CySwingAppAdapter adapter) {
                m_adapter = adapter;
        }

        public boolean install_protocol(LoaderProtocol protocol) {
                System.out.println(getClass() + " - Installing loader protocol: " + protocol + "...");
                BasicCyFileFilter filter = new BasicCyFileFilter(protocol.get_file_extension(),
                                                                 protocol.get_file_content_type(),
                                                                 protocol.get_file_description(),
                                                                 protocol.get_file_category(),
                                                                 m_adapter.getStreamUtil());
                LoaderInputStreamFactory input_stream = new LoaderInputStreamFactory(protocol, filter, this);
                Properties props = new Properties();
                props.setProperty("readerDescription", protocol.get_file_description());
                props.setProperty("readerId", protocol.get_file_loader_id());
                m_adapter.getCyServiceRegistrar().registerService(input_stream, InputStreamTaskFactory.class, props);
                return true;
        }

        public CyNetworkFactory get_network_factory() {
                return m_adapter.getCyNetworkFactory();
        }

        public CyNetworkViewFactory get_network_view_factory() {
                return m_adapter.getCyNetworkViewFactory();
        }

        public CyTableFactory get_table_factory() {
                return m_adapter.getCyTableFactory();
        }
}
