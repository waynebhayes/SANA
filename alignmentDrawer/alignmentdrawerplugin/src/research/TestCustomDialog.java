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

import java.awt.Container;
import javax.swing.BoxLayout;
import javax.swing.JButton;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.SwingConstants;

class SimpleFrameDialog extends CustomDialog {
        
        private String  m_input_string = "";
        private String  m_title = "";
        
        private JTextArea m_text0;
        private JTextArea m_text1;
        private JButton m_confirm;
        
        public SimpleFrameDialog() { 
        }
        
        @Override
        public void set_data(Object data) {
                m_title = (String) data;
                
                System.out.println("Setting title: " + m_title);
//                super.get_container().setTitle(m_title);
        }

        @Override
        public Object get_data() {
                m_input_string = "text0: " + m_text0.getText() + " \ntext1: " + m_text1.getText();
                return m_input_string;
        }

        @Override
        public String get_name() {
                return getClass().toString();
        }

        @Override
        public void run() {
                Container dialog = super.get_container();
                setTitle(m_title);
                dialog.setSize(300, 300);
             
                
                m_text0 = new JTextArea();
                m_text1 = new JTextArea();
                
                JPanel left_panel = new JPanel();
                JPanel right_panel = new JPanel();
                
                left_panel.add(new JLabel("g0 Nodes"));
                left_panel.add(new JScrollPane(m_text0));
                left_panel.setLayout(new BoxLayout(left_panel, BoxLayout.Y_AXIS));
                
                right_panel.add(new JLabel("g1 Nodes"));
                right_panel.add(new JScrollPane(m_text1));
                right_panel.setLayout(new BoxLayout(right_panel, BoxLayout.Y_AXIS));
                
                JPanel top_panel = new JPanel();
                top_panel.add(left_panel);
                top_panel.add(right_panel);
                top_panel.setLayout(new BoxLayout(top_panel, BoxLayout.X_AXIS));
                
                m_confirm = new JButton("Confirm");
                m_confirm.setHorizontalAlignment(SwingConstants.TRAILING);
                
                Container cont = dialog;
                JPanel univ_cont = new JPanel();
                univ_cont.setLayout(new BoxLayout(univ_cont, BoxLayout.Y_AXIS));
                univ_cont.add(top_panel);
                univ_cont.add(m_confirm);
                
                set_container(univ_cont);
        }
}

class AlignmentSwitchDialog extends CustomDialog {

        @Override
        public void run() {
                UIGetSwitchNodes ui = new UIGetSwitchNodes();
                set_container(ui);
        }

        @Override
        public void set_data(Object data) {
        }

        @Override
        public Object get_data() {
                return null;
        }

        @Override
        public String get_name() {
                return getClass().toString();
        }
        
}

/**
 * To test the running of customized UI.
 * @author davis
 */
public class TestCustomDialog implements Test {

        @Override
        public boolean test() {
                SimpleFrameDialog dialog = new SimpleFrameDialog();
                String input = (String) Util.run_customized_dialog(dialog, "Simple Dialog");
                System.out.println(getClass().toString() + " - input: " + input);
                
                AlignmentSwitchDialog dialog2 = new AlignmentSwitchDialog();
                Object result = Util.run_customized_dialog(dialog2, null);
                return true;
        }

        @Override
        public String name() {
                return "Custom Dialog Test";
        }

        @Override
        public String failure_reason() {
                return null;
        }
        
}
