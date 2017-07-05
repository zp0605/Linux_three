function validate(){
	var oEditor = FCKeditorAPI.GetInstance('pagetext'); 
	var oDOM = oEditor.EditorDocument; 
	var iLength ; 
	if(document.all){ 
		iLength = trim(oDOM.body.innerText).length; 
	}else{ 
		var r = oDOM.createRange(); 
		r.selectNodeContents(oDOM.body); 
		iLength = r.toString().length; 
	} 
	var oEditor=FCKeditorAPI.GetInstance('pagetext');
	if(document.getElementById('txtqqqq').value==""){
		alert('请填写标题！');
		return false;
	}else if(iLength < 5){
		alert('内容不能少于5个字符！');
		return false;
	}else{
		return true;
	}
}
function trim(str){ //删除左右两端的空格
 　　     return str.replace(/(^\s*)|(\s*$)/g, "");
}
 