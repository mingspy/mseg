import java.io.*;
import com.mingspy.mseg.*;
import java.util.*;

public class Speed {  
        public static void main(String[] args) throws InterruptedException, FileNotFoundException {  
            MsegJNI.init();
            System.err.println("-------------call seg-----------------");
            List<String> lines = new ArrayList<String>();
            double bytes = 0;
            FileInputStream fis = null;
            InputStreamReader isr = null;
            BufferedReader br = null; //用于包装InputStreamReader,提高处理性能。因为BufferedReader有缓冲的，而InputStreamReader没有。
            try {
                String line = null;
                 fis = new FileInputStream("../test_data.txt");// FileInputStream // 从文件系统中的某个文件中获取字节
                 isr = new InputStreamReader(fis);// InputStreamReader 是字节流通向字符流的桥梁,
                 br = new BufferedReader(isr);// 从字符输入流中读取文件中的内容,封装了一个new InputStreamReader的对象
                 while ((line = br.readLine()) != null) {
                     lines.add(line);
                     bytes += line.length();
                 }
                 long start_time = System.currentTimeMillis();
                 for( String str : lines){
                     List<Token> ls = MsegJNI.FullSplit(str);
                 }
                 double used_time = System.currentTimeMillis() - start_time;
                 System.out.println("full split speed is "+(bytes / used_time / 1024.0 / 1024.0 * 1000));
                 start_time = System.currentTimeMillis();
                 for( String str : lines){
                     List<Token> ls = MsegJNI.BackwardSplit(str);
                 }
                 used_time = System.currentTimeMillis() - start_time;
                 System.out.println("backward split speed is "+(bytes / used_time / 1024.0 / 1024.0 * 1000));
                 start_time = System.currentTimeMillis();
                 for( String str : lines){
                     List<Token> ls = MsegJNI.ForwardSplit(str);
                 }
                 used_time = System.currentTimeMillis() - start_time;
                 System.out.println("forward split speed is "+(bytes / used_time / 1024.0 / 1024.0 * 1000));
                 start_time = System.currentTimeMillis();
                 for( String str : lines){
                     List<Token> ls = MsegJNI.SmartSplit(str);
                 }
                 used_time = System.currentTimeMillis() - start_time;
                 System.out.println("smart split speed is "+(bytes / used_time / 1024.0 / 1024.0 * 1000));
                 start_time = System.currentTimeMillis();
                 for( String str : lines){
                     List<Token> ls = MsegJNI.Tagging(str);
                 }
                 used_time = System.currentTimeMillis() - start_time;
                 System.out.println("tagging split speed is "+(bytes / used_time / 1024.0 / 1024.0 * 1000));
            }catch (FileNotFoundException e) {
                 System.out.println("找不到指定文件");
            } catch (IOException e) {    
                 System.out.println("读取文件失败"); 
            } finally { 
                 try { 
                     br.close();
                 } catch (IOException e) {
                     e.printStackTrace();
                 }
            } 
        }
} 
