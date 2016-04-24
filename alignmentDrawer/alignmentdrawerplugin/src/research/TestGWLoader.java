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

import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.InputStream;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 * Test LoaderGW with an actual file
 *
 * @author Wen, Chifeng <https://sourceforge.net/u/daviesx/profile/>
 */
public class TestGWLoader implements Test {

        LoaderGW m_gw_loader;
        InputStream m_is;
        String m_failure_reason = "";

        public TestGWLoader(String test_file) throws FileNotFoundException {
                m_is = new FileInputStream(test_file);
                m_gw_loader = new LoaderGW();
        }

        @Override
        public boolean test() {
                try {
                        m_gw_loader.set_input_stream(m_is);
                        m_gw_loader.run(null);
                        return true;
                } catch (Exception ex) {
                        m_failure_reason = ex.getMessage();
                        Logger.getLogger(TestGWLoader.class.getName()).log(Level.SEVERE, null, ex);
                }
                return false;
        }

        @Override
        public String name() {
                return this.toString();
        }

        @Override
        public String failure_reason() {
                return m_failure_reason;
        }
}
