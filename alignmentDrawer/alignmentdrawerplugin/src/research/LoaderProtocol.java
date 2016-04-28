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
import java.util.Set;
import org.cytoscape.io.DataCategory;
import org.cytoscape.work.Task;

/**
 * Set up a protocol for all cytoscape file loader
 *
 * @author Wen, Chifeng <https://sourceforge.net/u/daviesx/profile/>
 */
public interface LoaderProtocol extends Task {

        public void set_input_stream(InputStream s);
//        public void             set_network_factory(CyNetworkFactory fact);
//        public void             set_network_view_factory(CyNetworkViewFactory fact);
//        public void             set_table_factory(CyTableFactory fact);
        
        public void set_file_name(String filename);

        public void set_loader_service(CytoscapeLoaderService service);

        public Set<String> get_file_extension();

        public Set<String> get_file_content_type();

        public DataCategory get_file_category();

        public String get_file_description();

        public String get_file_loader_id();
}
