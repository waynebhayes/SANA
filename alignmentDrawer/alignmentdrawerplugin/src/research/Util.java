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

import java.awt.Component;
import java.awt.event.ComponentEvent;
import java.awt.event.ComponentListener;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.io.BufferedReader;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.StringWriter;
import java.util.HashSet;
import java.util.List;
import java.util.Set;
import java.util.logging.Level;
import java.util.logging.Logger;
import javax.swing.JFileChooser;
import javax.swing.JFrame;
import javax.swing.JOptionPane;
import javax.swing.filechooser.FileNameExtensionFilter;
import org.cytoscape.work.TaskMonitor;
import static research.Util.LOCK;

class DialogRunner extends Thread {

        private CustomDialog m_dialog = null;
        
        class DialogListener implements ComponentListener {

                @Override
                public void componentResized(ComponentEvent e) {
                }

                @Override
                public void componentMoved(ComponentEvent e) {
                }

                @Override
                public void componentShown(ComponentEvent e) {
                }

                @Override
                public void componentHidden(ComponentEvent e) {
                        synchronized (LOCK) {
                                m_dialog.setVisible(false);
                                LOCK.notify();
                        }
                }
                
        }

        public DialogRunner(CustomDialog dialog) {
                m_dialog = dialog;
                m_dialog.addComponentListener(new DialogListener());
                m_dialog.setDefaultCloseOperation(JFrame.HIDE_ON_CLOSE);
        }

        @Override
        public void run() {
                System.out.println(getClass() + " - running " + m_dialog.get_name());
                synchronized (LOCK) {
                        m_dialog.run();
                        m_dialog.pack();
                        m_dialog.setVisible(true);
                                        
                        while (m_dialog.isVisible()) {
                                try {
                                        LOCK.wait();
                                } catch (InterruptedException e) {
                                }
                        }
                        System.out.println(getClass() + " - finished " + m_dialog.get_name());
                }
        }
}

/**
 * Utility class
 *
 * @author Wen, Chifeng <https://sourceforge.net/u/daviesx/profile/>
 */
public class Util {

        private static final int MAX_BUFFERING = 16384;

        public static String extract_string_from_stream(InputStream source) throws IOException {
                StringWriter writer = new StringWriter();
                try (BufferedReader reader = new BufferedReader(new InputStreamReader(source))) {
                        char[] buffer = new char[MAX_BUFFERING];
                        int n_bytes = reader.read(buffer, 0, buffer.length);
                        while (n_bytes != -1) {
                                writer.write(buffer, 0, n_bytes);
                                n_bytes = reader.read(buffer, 0, buffer.length);
                        }
                }
                return writer.toString();
        }

        public static String[] extract_lines_from_stream(InputStream source) throws IOException {
                return extract_string_from_stream(source).split("\n");
        }

        public static String run_selection_dialog(String[] list_of_data, String title, String prompt) {
                String default_string = "";
                if (list_of_data.length != 0) {
                        default_string = list_of_data[0];
                }
                return (String) JOptionPane.showInputDialog(null, title, prompt,
                                                            JOptionPane.PLAIN_MESSAGE, null,
                                                            list_of_data, default_string);
        }
        
        public static String run_input_dialog(String title, String prompt) {
                return JOptionPane.showInputDialog(null,  prompt, title, JOptionPane.PLAIN_MESSAGE);
        }

        static final Object LOCK = new Object();

        public static Object run_customized_dialog(final CustomDialog dialog, Object data) {
                dialog.set_data(data);
                
                DialogRunner runner = new DialogRunner(dialog);
                Thread t = new DialogRunner(dialog);
                t.start();
                try {
                        t.join();
                } catch (InterruptedException ex) {
                        Logger.getLogger(Util.class.getName()).log(Level.SEVERE, null, ex);
                }
                return dialog.get_data();
        }

        public static void prompt_critical_error(String title, String prompt) {
                JOptionPane.showMessageDialog(null, prompt, title, JOptionPane.ERROR_MESSAGE);
        }

        public static File run_file_chooser(String file_desc, String extension) {
                JFileChooser chooser = new JFileChooser();
                FileNameExtensionFilter filter = new FileNameExtensionFilter(file_desc, extension);
                chooser.setFileFilter(filter);
                int ret_val = chooser.showOpenDialog(null);
                if (ret_val == JFileChooser.APPROVE_OPTION) {
                        return chooser.getSelectedFile();
                } else {
                        return null;
                }
        }

        public static <T> Set<T> list_to_set(List<T> l) {
                HashSet<T> set = new HashSet<>(l.size());
                for (T e : l) {
                        set.add(e);
                }
                return set;
        }

        public static void advance_progress(TaskMonitor tm, Integer current, Integer total) {
                if (tm != null) {
                        tm.setProgress((double) current / total);
                }
                current++;
        }
}
