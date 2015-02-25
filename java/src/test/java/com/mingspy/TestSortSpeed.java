package com.mingspy;

import java.util.Arrays;
import java.util.Random;

import org.junit.Test;

public class TestSortSpeed
{

    @Test
    public void intSort()
    {
        Random r = new Random();
        final int n = 10000000;
        int []arr = new int[n];
        for(int i = 0; i< n; i++) {
            arr[i] = r.nextInt();
        }

        long start_time = System.currentTimeMillis();
        Arrays.sort(arr);
        long end_time = System.currentTimeMillis();
        System.out.println("int sort used  " + (end_time - start_time));
        for(int i = 0; i < 10; i++) {
            System.out.println(arr[i]);
        }

    }

    @Test
    public void doubleSort()
    {
        Random r = new Random();
        final int n = 10000000;
        double []arr = new double[n];
        for(int i = 0; i< n; i++) {
            arr[i] = r.nextDouble();
        }

        long start_time = System.currentTimeMillis();
        Arrays.sort(arr);
        long end_time = System.currentTimeMillis();
        System.out.println("double sort used  " + (end_time - start_time));
        for(int i = 0; i < 10; i++) {
            System.out.println(arr[i]);
        }
    }
}
