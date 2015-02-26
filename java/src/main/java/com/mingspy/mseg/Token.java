package com.mingspy.mseg;

public class Token
{
    private int start;
    private int end;
    private String word;
    private String nature;
    
    public Token(int start, int end)
    {
        this(start,end,null,null);
    }
    public Token(int start, int end, String word, String nature)
    {
        this.start = start;
        this.end = end;
        this.word = word;
        this.nature = nature;
    }
    
    public int getStart() {
		return start;
	}
	public void setStart(int start) {
		this.start = start;
	}
	public int getEnd() {
		return end;
	}
	public void setEnd(int end) {
		this.end = end;
	}
	public String getWord() {
		return word;
	}
	public void setWord(String word) {
		this.word = word;
	}
	public String getNature() {
		if(nature != null){
			return nature;
		}
		return "word";
	}
	public void setNature(String nature) {
		this.nature = nature;
	}
	
    @Override
    public String toString()
    {
        StringBuilder builder = new StringBuilder();
        builder.append("(");
        builder.append(start);
        builder.append(",");
        builder.append(end);
        if(word != null) {
        	builder.append(",");
            builder.append(word);
        }
        if(nature != null) {
            builder.append("/");
            builder.append(nature);
        }
        builder.append(")");
        return builder.toString();
    }
	public int length() {
		return end - start;
	}

}
