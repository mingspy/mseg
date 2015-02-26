package com.mingspy.lucene;

import java.io.IOException;
import java.io.Reader;
import java.util.Iterator;
import java.util.List;

import org.apache.lucene.analysis.Tokenizer;
import org.apache.lucene.analysis.tokenattributes.CharTermAttribute;
import org.apache.lucene.analysis.tokenattributes.OffsetAttribute;
import org.apache.lucene.analysis.tokenattributes.PositionIncrementAttribute;
import org.apache.lucene.analysis.tokenattributes.TypeAttribute;

import com.mingspy.mseg.MsegJNI;
import com.mingspy.mseg.Token;

public class MsegTokenizer extends Tokenizer {
	public final static int FORWARD = 0x01;
	public final static int BACKWARD = 0x02;
	public final static int SMART = 0x04;
	public final static int TAGGING = 0x08;
	public final static int FULL = 0x10;
	// 词元文本属性
	private final CharTermAttribute termAtt;
	// 词元位移属性
	private final OffsetAttribute offsetAtt;
	// 词元分类属性
	private final TypeAttribute typeAtt;
	private final PositionIncrementAttribute posIncrAtt;
	private Iterator<Token> _tokens;
	private char[] _charBuf = new char[8192];
	private int splitMethod = FORWARD;

	public MsegTokenizer(Reader input) {
		super(input);
		offsetAtt = addAttribute(OffsetAttribute.class);
		termAtt = addAttribute(CharTermAttribute.class);
		typeAtt = addAttribute(TypeAttribute.class);
		posIncrAtt = addAttribute(PositionIncrementAttribute.class);
	}

	private void splitText(Reader input) {
		StringBuilder sb = new StringBuilder();
		int d = -1;
		try {
			while ((d = input.read(_charBuf)) != -1) {
				sb.append(_charBuf, 0, d);
			}
			String str = sb.toString();
			List<Token> _splitResult = null;
			if (!str.isEmpty()) {
				switch (splitMethod) {
				case BACKWARD:
					_splitResult = MsegJNI.BackwardSplit(str);
					break;
				case SMART:
					_splitResult = MsegJNI.SmartSplit(str);
					break;
				case TAGGING:
					_splitResult = MsegJNI.Tagging(str);
					break;
				case FULL:
					_splitResult = MsegJNI.FullSplit(str);
					break;
				default:
					_splitResult = MsegJNI.ForwardSplit(str);
				}
			}
			_tokens = _splitResult != null ? _splitResult.iterator() : null;
		} catch (Exception e) {
			_tokens = null;
			e.printStackTrace();
		}
	}

	@Override
	public boolean incrementToken() throws IOException {
		clearAttributes();
		Token token = null;
		if (_tokens == null) {
			splitText(input);
		}
		
		if (_tokens != null && _tokens.hasNext()) {
			token = _tokens.next();
			posIncrAtt.setPositionIncrement(1);
			//将Lexeme转成Attributes
			//设置词元文本
			termAtt.append(token.getWord());
			//设置词元长度
			termAtt.setLength(token.length());
			//设置词元位移
            offsetAtt.setOffset(correctOffset(token.getStart()), correctOffset(token.getEnd()));
			//记录词元分类
			typeAtt.setType(token.getNature());			
			//返会true告知还有下个词元
			return true;
		}

		return false;
	}

	@Override
	public void reset() throws IOException {
		super.reset();
		_tokens = null;
	}

	public void setSplitMethod(int splitMethod) {
		this.splitMethod = splitMethod;
	}

}
