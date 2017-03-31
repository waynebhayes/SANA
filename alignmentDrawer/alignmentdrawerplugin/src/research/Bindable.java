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

/**
 * Base class that describle something binded to another object
 *
 * @author davis
 */
public class Bindable {

        private final Object m_to_bind;
        private final String m_id;

        Bindable(Object to_bind) {
                m_to_bind = to_bind;
                m_id = "";
        }

        Bindable(Object to_bind, String id) {
                m_to_bind = to_bind;
                m_id = id;
        }

        String get_id() {
                return m_id;
        }

        Object get_binded() {
                return m_to_bind;
        }
}
